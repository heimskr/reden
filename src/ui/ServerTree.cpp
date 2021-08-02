#include "UI.h"
#include "core/Client.h"
#include "ui/ServerTree.h"

#include "pingpong/commands/Part.h"
#include "pingpong/core/Channel.h"
#include "pingpong/core/Server.h"
#include "pingpong/core/User.h"

namespace Reden {
	ServerTree::ServerTree(Client &client_): Gtk::TreeView(), client(client_) {
		getActiveView = [] { return nullptr; };
		model = Gtk::TreeStore::create(columns);
		set_model(model);
		set_vexpand(true);
		set_headers_visible(false);
		set_activate_on_single_click(true);
		set_can_focus(false);
		add_css_class("server-tree");
		appendColumn(*this, "Name", columns.name);
		signal_cursor_changed().connect(sigc::mem_fun(*this, &ServerTree::cursorChanged));
		addStatusRow();

		auto gesture = Gtk::GestureClick::create();
		gesture->set_button(GDK_BUTTON_SECONDARY);
		gesture->signal_pressed().connect(sigc::mem_fun(*this, &ServerTree::secondaryClicked), true);
		add_controller(gesture);

		auto menu = Gio::Menu::create();
		menu->append("_Disconnect", "popup.close");
		menu->append("_Clear", "popup.clear");
		popupMenuServer.set_parent(*this);
		popupMenuServer.set_menu_model(menu);

		menu = Gio::Menu::create();
		menu->append("_Leave", "popup.close");
		menu->append("_Clear", "popup.clear");
		popupMenuChannel.set_parent(*this);
		popupMenuChannel.set_menu_model(menu);

		menu = Gio::Menu::create();
		menu->append("_Close", "popup.close");
		menu->append("C_lear", "popup.clear");
		popupMenuUser.set_parent(*this);
		popupMenuUser.set_menu_model(menu);

		auto group = Gio::SimpleActionGroup::create();
		group->add_action("close", sigc::mem_fun(*this, &ServerTree::close));
		group->add_action("clear", sigc::mem_fun(*this, &ServerTree::clear));
		insert_action_group("popup", group);
	}

	void ServerTree::addStatusRow() {
		auto row = model->append();
		(*row)[columns.name] = "Status";
		(*row)[columns.pointer] = this;
		(*row)[columns.type] = Type::Status;
		serverRows.emplace(this, row);
		get_selection()->select(row);
	}

	void ServerTree::add(PingPong::Server *server, bool focus) {
		if (serverRows.count(server) != 0) {
			if (focus)
				focusView(server);
			return;
		}
		auto row = model->append();
		(*row)[columns.name] = server->id;
		(*row)[columns.pointer] = server;
		(*row)[columns.type] = Type::Server;
		serverRows.emplace(server, row);
		if (focus)
			focusView(server);
	}

	void ServerTree::add(PingPong::Channel *channel, bool focus) {
		if (channelRows.count(channel) != 0) {
			if (focus)
				focusView(channel);
			return;
		}
		if (serverRows.count(channel->server) == 0)
			add(channel->server, false);
		auto iter = serverRows.at(channel->server);
		auto row = model->append(iter->children());
		expand_row(Gtk::TreeModel::Path(iter), false);
		(*row)[columns.name] = channel->name;
		(*row)[columns.pointer] = channel;
		(*row)[columns.type] = Type::Channel;
		channelRows.emplace(channel, row);
		if (focus)
			focusView(channel);
	}

	void ServerTree::add(PingPong::User *user, bool focus) {
		if (userRows.count(user) != 0) {
			if (focus)
				focusView(user);
			return;
		}
		if (serverRows.count(user->server) == 0)
			add(user->server, false);
		auto iter = serverRows.at(user->server);
		auto row = model->append(iter->children());
		expand_row(Gtk::TreeModel::Path(iter), false);
		(*row)[columns.name] = user->name;
		(*row)[columns.pointer] = user;
		(*row)[columns.type] = Type::User;
		userRows.emplace(user, row);
		if (focus)
			focusView(user);
	}

	void ServerTree::erase(PingPong::Server *server) {
		if (serverRows.count(server) == 0)
			return;

		std::vector<PingPong::Channel *> remove_channels;
		remove_channels.reserve(channelRows.size());
		for (auto &[channel, iter]: channelRows)
			if (channel->server == server)
				remove_channels.push_back(channel);
		for (PingPong::Channel *channel: remove_channels)
				erase(channel);

		std::vector<PingPong::User *> remove_users;
		remove_users.reserve(userRows.size());
		for (auto &[user, iter]: userRows)
			if (user->server == server)
				remove_users.push_back(user);
		for (PingPong::User *user: remove_users)
			erase(user);

		model->erase(serverRows.at(server));
		serverRows.erase(server);
		signal_erase_requested_.emit(server);
		if (getActiveView() == server)
			signal_status_focus_requested_.emit();
	}

	void ServerTree::erase(PingPong::Channel *channel) {
		if (channelRows.count(channel) == 0)
			return;
		model->erase(channelRows.at(channel));
		channelRows.erase(channel);
		signal_erase_requested_.emit(channel);
		if (getActiveView() == channel)
			focusView(channel->server);
	}

	void ServerTree::erase(PingPong::User *user) {
		if (userRows.count(user) == 0)
			return;
		model->erase(userRows.at(user));
		userRows.erase(user);
		signal_erase_requested_.emit(user);
		if (getActiveView() == user)
			focusView(user->server);
	}

	void ServerTree::cursorChanged() {
		if (auto iter = get_selection()->get_selected())
			focusVoid((*iter)[columns.pointer]);
	}

	Gtk::TreeModel::Path ServerTree::getPath(const Gtk::TreeModel::const_iterator &iter) {
		return model->get_path(iter);
	}

	void ServerTree::focusView(PingPong::Channel *channel) {
		signal_channel_focus_requested_.emit(channel);
	}

	void ServerTree::focusView(PingPong::Server *server) {
		signal_server_focus_requested_.emit(server);
	}

	void ServerTree::focusView(PingPong::User *user) {
		signal_user_focus_requested_.emit(user);
	}

	void ServerTree::focusVoid(void *ptr) {
		signal_focus_requested_.emit(ptr);
	}

	void ServerTree::secondaryClicked(int, double x, double y) {
		Gtk::TreeModel::Path path;
		Gtk::TreeView::DropPosition pos;
		get_dest_row_at_pos(x, y, path, pos);
		if (path.empty())
			return;
		switch ((*model->get_iter(path))[columns.type]) {
			case Type::Server:
				popupMenuServer.set_pointing_to(Gdk::Rectangle(x, y, 1, 1));
				popupMenuServer.popup();
				break;
			case Type::Channel:
				popupMenuChannel.set_pointing_to(Gdk::Rectangle(x, y, 1, 1));
				popupMenuChannel.popup();
				break;
			case Type::User:
				popupMenuUser.set_pointing_to(Gdk::Rectangle(x, y, 1, 1));
				popupMenuUser.popup();
				break;
			default:
				break;
		}
	}

	void ServerTree::close() {
		if (auto iter = get_selection()->get_selected())
			switch ((*iter)[columns.type]) {
				case Type::Server: {
					auto *server = reinterpret_cast<PingPong::Server *>((void *) ((*iter)[columns.pointer]));
					server->quit();
					erase(server);
					break;
				}
				case Type::Channel: {
					auto *channel = reinterpret_cast<PingPong::Channel *>((void *) ((*iter)[columns.pointer]));
					PingPong::PartCommand(channel).send();
					break;
				}
				case Type::User:
					erase(reinterpret_cast<PingPong::User *>((void *) ((*iter)[columns.pointer])));
					break;
				default:
					break;
			}
	}

	void ServerTree::clear() {
		if (auto iter = get_selection()->get_selected())
			signal_clear_requested_.emit((*iter)[columns.pointer]);
	}

	sigc::signal<void()> ServerTree::signal_status_focus_requested() {
		return signal_status_focus_requested_;
	}

	sigc::signal<void(PingPong::Channel *)> ServerTree::signal_channel_focus_requested() {
		return signal_channel_focus_requested_;
	}

	sigc::signal<void(PingPong::Server *)> ServerTree::signal_server_focus_requested() {
		return signal_server_focus_requested_;
	}

	sigc::signal<void(PingPong::User *)> ServerTree::signal_user_focus_requested() {
		return signal_user_focus_requested_;
	}

	sigc::signal<void(void *)> ServerTree::signal_focus_requested() {
		return signal_focus_requested_;
	}

	sigc::signal<void(void *)> ServerTree::signal_erase_requested() {
		return signal_erase_requested_;
	}

	sigc::signal<void(void *)> ServerTree::signal_clear_requested() {
		return signal_clear_requested_;
	}
}
