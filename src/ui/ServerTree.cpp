#include "UI.h"
#include "ui/ServerTree.h"

#include "pingpong/core/Channel.h"
#include "pingpong/core/Server.h"
#include "pingpong/core/User.h"

namespace Reden {
	ServerTree::ServerTree(): Gtk::TreeView() {
		model = Gtk::TreeStore::create(columns);
		set_model(model);
		set_vexpand(true);
		set_headers_visible(false);
		set_activate_on_single_click(true);
		set_can_focus(false);
		add_css_class("server-tree");
		appendColumn(*this, "Name", columns.name);
		signal_row_activated().connect(sigc::mem_fun(*this, &ServerTree::rowActivated));
		signal_cursor_changed().connect(sigc::mem_fun(*this, &ServerTree::cursorChanged));
		addStatusRow();
	}

	void ServerTree::addStatusRow() {
		auto row = model->append();
		(*row)[columns.name] = "Status";
		(*row)[columns.pointer] = this;
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
		userRows.emplace(user, row);
		if (focus)
			focusView(user);
	}

	void ServerTree::erase(PingPong::Server *server, void *active_view) {
		if (serverRows.count(server) == 0)
			return;

		std::vector<PingPong::Channel *> remove_channels;
		remove_channels.reserve(channelRows.size());
		for (auto &[channel, iter]: channelRows)
			if (channel->server == server)
				remove_channels.push_back(channel);
		for (PingPong::Channel *channel: remove_channels)
				erase(channel, active_view);

		std::vector<PingPong::User *> remove_users;
		remove_users.reserve(userRows.size());
		for (auto &[user, iter]: userRows)
			if (user->server == server)
				remove_users.push_back(user);
		for (PingPong::User *user: remove_users)
			erase(user, active_view);

		model->erase(serverRows.at(server));
		serverRows.erase(server);
		signal_erase_requested_.emit(server);
		if (active_view == server)
			signal_status_focus_requested_.emit();
	}

	void ServerTree::erase(PingPong::Channel *channel, void *active_view) {
		if (channelRows.count(channel) == 0)
			return;
		model->erase(channelRows.at(channel));
		channelRows.erase(channel);
		signal_erase_requested_.emit(channel);
		if (active_view == channel)
			focusView(channel->server);
	}

	void ServerTree::erase(PingPong::User *user, void *active_view) {
		if (userRows.count(user) == 0)
			return;
		model->erase(userRows.at(user));
		userRows.erase(user);
		signal_erase_requested_.emit(user);
		if (active_view == user)
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

	void ServerTree::rowActivated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *) {
		if (auto iter = model->get_iter(path))
			focusVoid((*iter)[columns.pointer]);
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
}
