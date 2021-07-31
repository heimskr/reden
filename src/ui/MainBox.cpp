#include "UI.h"
#include "ui/MainBox.h"
#include "ui/RedenWindow.h"

#include "pingpong/commands/Privmsg.h"
#include "pingpong/core/Debug.h"
#include "pingpong/core/Server.h"

#include "VSCHacks.h"

namespace Reden {
	MainBox::MainBox(RedenWindow &parent_): Gtk::Box(Gtk::Orientation::HORIZONTAL), parent(parent_) {
		serverModel = Gtk::TreeStore::create(columns);
		serverTree.set_model(serverModel);
		serverTree.set_vexpand(true);
		serverTree.set_headers_visible(false);
		serverTree.set_activate_on_single_click(true);
		serverTree.set_can_focus(false);
		serverTree.add_css_class("server-tree");
		appendColumn(serverTree, "Name", columns.name);
		chatBox.set_expand(true);
		userModel = Gtk::ListStore::create(columns);
		userModel->set_sort_func(columns.name, sigc::mem_fun(*this, &MainBox::compareUsers));
		userTree.set_model(userModel);
		userTree.set_vexpand(true);
		userTree.set_headers_visible(false);
		userTree.set_activate_on_single_click(true);
		userTree.set_can_focus(false);
		userTree.add_css_class("user-tree");
		appendColumn(userTree, "Name", columns.name);
		serversScrolled.set_child(serverTree);
		usersScrolled.set_child(userTree);
		append(serversScrolled);
		append(leftSeparator);
		append(chatBox);
		append(rightSeparator);
		append(usersScrolled);
		serversScrolled.set_size_request(200, -1);
		usersScrolled.set_size_request(200, -1);
		topicScrolled.set_child(topicLabel);
		topicScrolled.set_margin(0);
		topicLabel.set_margin_start(5);
		topicLabel.set_margin_end(5);
		chatBox.append(topicScrolled);
		chatBox.append(topicSeparator);
		chatBox.append(chatScrolled);
		chatBox.append(chatEntry);
		chatEntry.add_css_class("unrounded");
		chatScrolled.set_vexpand(true);
		chatEntry.signal_activate().connect(sigc::mem_fun(*this, &MainBox::entryActivated));
		chatEntry.grab_focus();
		addStatusRow();
		serverTree.signal_row_activated().connect(sigc::mem_fun(*this, &MainBox::serverRowActivated));
		keyController = Gtk::EventControllerKey::create();;
		keyController->signal_key_pressed().connect(sigc::mem_fun(*this, &MainBox::keyPressed), false);
		add_controller(keyController);
	}

	Client & MainBox::client() {
		return parent.client;
	}

	void MainBox::addStatusRow() {
		auto row = serverModel->append();
		(*row)[columns.name] = "Status";
		(*row)[columns.pointer] = this;
		serverRows.emplace(this, row);
	}

	void MainBox::focusEntry() {
		chatEntry.grab_focus();
	}

	void MainBox::addServer(PingPong::Server *server, bool focus) {
		if (serverRows.count(server) != 0) {
			if (focus)
				focusView(server);
			return;
		}
		auto row = serverModel->append();
		(*row)[columns.name] = server->id;
		(*row)[columns.pointer] = server;
		serverRows.emplace(server, row);
		if (focus)
			focusView(server, server);
	}

	void MainBox::addChannel(PingPong::Channel *channel, bool focus) {
		if (channelRows.count(channel) != 0)
			return;
		if (serverRows.count(channel->server) == 0)
			addServer(channel->server, false);
		auto iter = serverRows.at(channel->server);
		auto row = serverModel->append(iter->children());
		serverTree.expand_row(Gtk::TreeModel::Path(iter), false);
		(*row)[columns.name] = channel->name;
		(*row)[columns.pointer] = channel;
		channelRows.emplace(channel, row);
		if (focus)
			focusView(channel, channel);
	}

	void MainBox::addUser(PingPong::User *user, bool focus) {
		if (userRows.count(user) != 0)
			return;
		if (serverRows.count(user->server) == 0)
			addServer(user->server, false);
		auto iter = serverRows.at(user->server);
		auto row = serverModel->append(iter->children());
		serverTree.expand_row(Gtk::TreeModel::Path(iter), false);
		(*row)[columns.name] = user->name;
		(*row)[columns.pointer] = user;
		userRows.emplace(user, row);
		if (focus)
			focusView(user, user);
	}

	void MainBox::eraseServer(PingPong::Server *server) {
		if (serverRows.count(server) == 0)
			return;
		serverModel->erase(serverRows.at(server));
		serverRows.erase(server);
	}

	void MainBox::eraseChannel(PingPong::Channel *channel) {
		if (channelRows.count(channel) == 0)
			return;
		serverModel->erase(channelRows.at(channel));
		channelRows.erase(channel);
	}

	void MainBox::addStatus(const std::string &line) {
		getLineView(this) += line;
	}

	void MainBox::updateChannel(PingPong::Channel &channel) {
		auto lock = channel.lockUsers();
		userSets[&channel].clear();
		if (activeView == &channel) {
			bool sort = false;
			for (auto &user: channel.users) {
				const auto highest = channel.getHats(user).highest();
				if (highest == PingPong::Hat::None)
					userSets[&channel].insert(user->name);
				else
					userSets[&channel].insert(user->name);
				if (presentUserRows.count(user->name) == 0) {
					auto row = userModel->append();
					presentUserRows.emplace(user->name, row);
					(*row)[columns.name] = channel.withHat(user, true);
					(*row)[columns.pointer] = &channel;
					sort = true;
				} else {
					auto row = presentUserRows.at(user->name);
					Glib::ustring new_name = channel.withHat(user, true);
					if (Glib::ustring((*row)[columns.name]) != new_name) {
						(*row)[columns.name] = new_name;
						sort = true;
					}
				}
			}

			if (sort)
				userModel->set_sort_column(columns.name, Gtk::SortType::ASCENDING);
		} else
			for (auto &user: channel.users)
				userSets[&channel].insert(user->name);
	}

	Glib::ustring MainBox::getInput() const {
		return chatEntry.get_text();
	}

	void MainBox::setInput(const Glib::ustring &text) {
		chatEntry.set_text(text);
	}

	int MainBox::getCursor() const {
		return chatEntry.get_position();
	}

	void MainBox::setCursor(int cursor) {
		chatEntry.set_position(cursor);
	}

	bool MainBox::inStatus() const {
		return activeView == this;
	}

	LineView & MainBox::active() {
		return getLineView(activeView);
	}

	PingPong::Server * MainBox::activeServer() {
		if (serverRows.count(activeView) != 0)
			return reinterpret_cast<PingPong::Server *>(activeView);
		// Ugly!
		PingPong::Channel *channel = reinterpret_cast<PingPong::Channel *>(activeView);
		if (channelRows.count(channel) != 0)
			return channel->server;
		return nullptr;
	}

	PingPong::Channel * MainBox::activeChannel() {
		PingPong::Channel *channel = reinterpret_cast<PingPong::Channel *>(activeView);
		if (channelRows.count(channel) != 0)
			return channel;
		return nullptr;
	}

	PingPong::User * MainBox::activeUser() {
		PingPong::User *user = reinterpret_cast<PingPong::User *>(activeView);
		if (userRows.count(user) != 0)
			return user;
		return nullptr;
	}

	void MainBox::log(const Glib::ustring &string) {
		active() += string;
	}

	LineView & MainBox::getLineView(void *ptr) {
		if (views.count(ptr) == 0)
			return views.emplace(ptr, LineView()).first->second;
		return views.at(ptr);
	}

	const LineView & MainBox::getLineView(void *ptr) const {
		return views.at(ptr);
	}

	LineView & MainBox::operator[](void *ptr) {
		if (views.count(ptr) == 0)
			return views.emplace(ptr, LineView()).first->second;
		return views.at(ptr);
	}

	const LineView & MainBox::operator[](void *ptr) const {
		return views.at(ptr);
	}

	void MainBox::setTopic(void *ptr, const std::string &topic) {
		topics[ptr] = topic;
		if (activeView == ptr)
			topicLabel.set_text(topic);
	}

	void MainBox::focusView(void *ptr) {
		LineView *view;
		focusView(ptr, view);
	}

	void MainBox::focusView(void *ptr, LineView * &line_out) {
		if (topics.count(ptr) != 0)
			topicLabel.set_text(topics.at(ptr));
		else
			topicLabel.set_text("");
		activeView = ptr;
		LineView &view = getLineView(ptr);
		line_out = &view;
		chatScrolled.set_child(view);
		userModel->clear();
		if (serverRows.count(ptr) != 0) {
			serverTree.get_selection()->select(serverRows.at(ptr));
			parent.irc->activeServer = reinterpret_cast<PingPong::Server *>(ptr);
		} else if (channelRows.count(reinterpret_cast<PingPong::Channel *>(ptr)) != 0) {
			PingPong::Channel *channel = reinterpret_cast<PingPong::Channel *>(ptr);
			topicLabel.set_text(topics[ptr] = std::string(channel->topic));
			serverTree.get_selection()->select(channelRows.at(channel));
			presentUserRows.clear();
			userModel->clear();
			for (const std::string &user: userSets[channel]) {
				auto row = userModel->append();
				presentUserRows[user] = row;
				(*row)[columns.name] = static_cast<char>(channel->getHats(user).highest()) + user;
				(*row)[columns.pointer] = channel;
			}
		} else if (userRows.count(reinterpret_cast<PingPong::User *>(ptr)) != 0) {
			PingPong::User *user = reinterpret_cast<PingPong::User *>(ptr);
			serverTree.get_selection()->select(userRows.at(user));
			presentUserRows.clear();
			userModel->clear();
			for (const std::string &user_str: {user->server->getSelf()->name, user->name}) {
				auto row = userModel->append();
				presentUserRows[user_str] = row;
				(*row)[columns.name] = user_str;
				(*row)[columns.pointer] = user;
			}
		}
	}

	void MainBox::serverRowActivated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *) {
		if (auto iter = serverModel->get_iter(path))
			focusView((*iter)[columns.pointer]);
	}

	int MainBox::compareUsers(const Gtk::TreeModel::const_iterator &left, const Gtk::TreeModel::const_iterator &right) {
		const Glib::ustring left_name = (*left)[columns.name], right_name = (*right)[columns.name];
		static const std::unordered_map<gunichar, int> priority {
			{'~', 0}, {'&', 1}, {'@', 2}, {'%', 3}, {'+', 4}, {' ', 5}
		};
		const int left_priority = priority.at(left_name[0]), right_priority = priority.at(right_name[0]);
		if (left_priority < right_priority)
			return -1;
		if (right_priority < left_priority)
			return 1;
		const Glib::ustring left_substr = left_name.substr(1), right_substr = right_name.substr(1);
		if (left_substr < right_substr)
			return -1;
		if (right_substr < left_substr)
			return 1;
		return 0;
	}

	bool MainBox::keyPressed(guint, guint keycode, Gdk::ModifierType) {
		if (keycode == 23 && chatEntry.get_focused()) { // tab
			parent.client.tabComplete();
			return true;
		}

		return false;
	}

	void MainBox::entryActivated() {
		// if (parent.irc->activeServer)
		// 	parent.irc->activeServer->quote(chatEntry.get_text());
		// else
		// 	std::cerr << "No active server.\n";
		// chatEntry.set_text("");

		const Glib::ustring input = chatEntry.get_text();
		chatEntry.set_text("");

		if (input.empty())
			return;

		InputLine il = client().getInputLine(input);
		// aliasDB.expand(il);

		// if (!beforeInput(il))
		// 	return;

		if (il.isCommand()) {
			try {
				if (!client().handleLine(il)) {
					// If the command isn't an exact match, try partial matches (e.g., "/j" for "/join").
					const Glib::ustring &cmd = il.command;

					std::vector<Glib::ustring> matches = client().commandMatches(cmd);

					if (1 < matches.size()) {
						log("Ambiguous command: /" + cmd);
						Glib::ustring joined;
						for (const Glib::ustring &match: matches)
							joined += "/" + match + " ";
						DBG("Matches: " << joined);
					} else if (matches.empty() || !client().handleLine("/" + matches[0] + " " + il.body)) {
						log("Unknown command: /" + cmd);
					}
				}
			} catch (std::exception &err) {
				log("Error: " + std::string(err.what()));
			}
		} else if (active().isAlive()) {
			if (PingPong::Channel *chan = activeChannel()) {
				PingPong::PrivmsgCommand(chan->server, chan->name, std::string(il.body)).send();
			} else if (PingPong::User *user = activeUser()) {
				PingPong::PrivmsgCommand(user->server, user->name, std::string(il.body)).send();
			} else {
				std::cout << "No channel.\n"; ///
			}
		}

		// afterInput(il);
	}
}
