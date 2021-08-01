#include "UI.h"
#include "ui/MainBox.h"
#include "ui/RedenWindow.h"

#include "pingpong/commands/Privmsg.h"
#include "pingpong/core/Debug.h"
#include "pingpong/core/Server.h"

#include "VSCHacks.h"

namespace Reden {
	MainBox::MainBox(RedenWindow &parent_): Gtk::Box(Gtk::Orientation::HORIZONTAL), parent(parent_) {
		serverTree = std::make_unique<ServerTree>(parent.client);
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
		serversScrolled.set_child(*serverTree);
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
		topicLabel.set_selectable(true);
		chatBox.append(topicScrolled);
		chatBox.append(topicSeparator);
		chatBox.append(chatScrolled);
		chatBox.append(chatEntry);
		chatEntry.add_css_class("unrounded");
		chatScrolled.set_vexpand(true);
		chatEntry.signal_activate().connect(sigc::mem_fun(*this, &MainBox::entryActivated));
		chatEntry.grab_focus();
		keyController = Gtk::EventControllerKey::create();;
		keyController->signal_key_pressed().connect(sigc::mem_fun(*this, &MainBox::keyPressed), false);
		add_controller(keyController);
		serverTree->getActiveView = [this] { return activeView; };
		serverTree->signal_status_focus_requested().connect([this] { focusView(&serverTree); });
		serverTree->signal_channel_focus_requested().connect([this](PingPong::Channel *channel) {
			focusView(channel, channel);
		});
		serverTree->signal_server_focus_requested().connect([this](PingPong::Server *server) {
			focusView(server, server);
		});
		serverTree->signal_user_focus_requested().connect([this](PingPong::User *user) {
			focusView(user, user);
		});
		serverTree->signal_focus_requested().connect([this](void *ptr) {
			focusView(ptr);
		});
		serverTree->signal_erase_requested().connect([this](void *ptr) { views.erase(ptr); });
	}

	Client & MainBox::client() {
		return parent.client;
	}

	void MainBox::focusEntry() {
		chatEntry.grab_focus();
	}

	void MainBox::addStatus(const std::string &line, bool pangoize) {
		getLineView(&serverTree).add(line, pangoize);
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
		return activeView == &serverTree;
	}

	LineView & MainBox::active() {
		return getLineView(activeView);
	}

	PingPong::Server * MainBox::activeServer() {
		if (serverTree->serverRows.count(activeView) != 0)
			return reinterpret_cast<PingPong::Server *>(activeView);
		// Ugly!
		PingPong::Channel *channel = reinterpret_cast<PingPong::Channel *>(activeView);
		if (serverTree->channelRows.count(channel) != 0)
			return channel->server;
		return nullptr;
	}

	PingPong::Channel * MainBox::activeChannel() {
		PingPong::Channel *channel = reinterpret_cast<PingPong::Channel *>(activeView);
		if (serverTree->channelRows.count(channel) != 0)
			return channel;
		return nullptr;
	}

	PingPong::User * MainBox::activeUser() {
		PingPong::User *user = reinterpret_cast<PingPong::User *>(activeView);
		if (serverTree->userRows.count(user) != 0)
			return user;
		return nullptr;
	}

	void MainBox::log(const Glib::ustring &string, bool pangoize) {
		active().add(string, pangoize);
	}

	bool MainBox::hasLineView(void *ptr) const {
		return views.count(ptr) != 0;
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

	void MainBox::add(PingPong::Channel *channel, bool focus) {
		serverTree->add(channel, focus);
	}

	void MainBox::add(PingPong::Server *server, bool focus) {
		serverTree->add(server, focus);
	}

	void MainBox::add(PingPong::User *user, bool focus) {
		serverTree->add(user, focus);
	}

	void MainBox::erase(PingPong::Channel *channel) {
		serverTree->erase(channel);
	}

	void MainBox::erase(PingPong::Server *server) {
		serverTree->erase(server);
	}

	void MainBox::erase(PingPong::User *user) {
		serverTree->erase(user);
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
		if (serverTree->serverRows.count(ptr) != 0) {
			serverTree->get_selection()->select(serverTree->serverRows.at(ptr));
			parent.irc->activeServer = reinterpret_cast<PingPong::Server *>(ptr);
		} else if (serverTree->channelRows.count(reinterpret_cast<PingPong::Channel *>(ptr)) != 0) {
			PingPong::Channel *channel = reinterpret_cast<PingPong::Channel *>(ptr);
			topicLabel.set_text(topics[ptr] = std::string(channel->topic));
			serverTree->get_selection()->select(serverTree->channelRows.at(channel));
			presentUserRows.clear();
			userModel->clear();
			for (const std::string &user: userSets[channel]) {
				auto row = userModel->append();
				presentUserRows[user] = row;
				(*row)[columns.name] = static_cast<char>(channel->getHats(user).highest()) + user;
				(*row)[columns.pointer] = channel;
			}
		} else if (serverTree->userRows.count(reinterpret_cast<PingPong::User *>(ptr)) != 0) {
			PingPong::User *user = reinterpret_cast<PingPong::User *>(ptr);
			serverTree->get_selection()->select(serverTree->userRows.at(user));
			presentUserRows.clear();
			userModel->clear();
			std::unordered_set<std::string> added_users;
			for (const std::string &user_str: {user->server->getSelf()->name, user->name}) {
				if (added_users.count(user_str) != 0)
					continue;
				added_users.insert(user_str);
				auto row = userModel->append();
				presentUserRows[user_str] = row;
				(*row)[columns.name] = " " + user_str;
				(*row)[columns.pointer] = user;
			}
		}
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

	bool MainBox::keyPressed(guint, guint keycode, Gdk::ModifierType modifiers) {
		if (keycode == 23 && chatEntry.get_focused()) { // tab
			parent.client.tabComplete();
			return true;
		}

		if (keycode == 111 && (modifiers & Gdk::ModifierType::ALT_MASK) == Gdk::ModifierType::ALT_MASK) { // up arrow
			if (auto iter = serverTree->get_selection()->get_selected()) {
				auto path = serverTree->getPath(iter);
				if (--iter) {
					serverTree->get_selection()->select(iter);
					serverTree->cursorChanged();
				} else if (1 < path.size() && path.up()) {
					serverTree->get_selection()->select(path);
					serverTree->cursorChanged();
				}
			}

			return true;
		}

		if (keycode == 116 && (modifiers & Gdk::ModifierType::ALT_MASK) == Gdk::ModifierType::ALT_MASK) { // down arrow
			if (auto iter = serverTree->get_selection()->get_selected()) {
				auto path = serverTree->getPath(iter);
				if (++iter) {
					serverTree->get_selection()->select(iter);
					serverTree->cursorChanged();
				} else {
					path.down();
					serverTree->get_selection()->select(path);
					serverTree->cursorChanged();
				}
			}

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
