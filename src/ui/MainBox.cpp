#include "UI.h"
#include "ui/MainBox.h"
#include "ui/RedenWindow.h"

#include "pingpong/core/Debug.h"
#include "pingpong/core/Server.h"

namespace Reden {
	MainBox::MainBox(RedenWindow &parent_): Gtk::Box(Gtk::Orientation::HORIZONTAL), parent(parent_) {
		serverModel = Gtk::TreeStore::create(serverColumns);
		serverTree.set_model(serverModel);
		serverTree.set_vexpand(true);
		serverTree.set_headers_visible(false);
		serverTree.set_activate_on_single_click(true);
		serverTree.set_size_request(200, -1);
		serverTree.set_can_focus(false);
		appendColumn(serverTree, "Name", serverColumns.name);
		chatBox.set_expand(true);
		userTree.set_vexpand(true);
		userTree.set_headers_visible(false);
		userTree.set_activate_on_single_click(true);
		userTree.set_size_request(200, -1);
		userTree.set_can_focus(false);
		append(serverTree);
		append(leftSeparator);
		append(chatBox);
		append(rightSeparator);
		append(userTree);
		topicScrolled.set_child(topicLabel);
		topicScrolled.set_margin(0);
		topicLabel.set_margin_start(5);
		topicLabel.set_margin_end(5);
		chatBox.append(topicScrolled);
		chatBox.append(topicSeparator);
		chatBox.append(scrolled);
		chatBox.append(chatEntry);
		chatEntry.add_css_class("unrounded");
		scrolled.set_vexpand(true);
		scrolled.set_child(chatGrid);
		chatEntry.signal_activate().connect([this] {
			if (parent.irc->activeServer)
				parent.irc->activeServer->quote(chatEntry.get_text());
			else
				std::cerr << "No active server.\n";
			chatEntry.set_text("");
		});
		chatEntry.grab_focus();
		addStatusRow();
		serverTree.signal_row_activated().connect(sigc::mem_fun(*this, &MainBox::serverRowActivated));
	}

	void MainBox::addStatusRow() {
		auto row = serverModel->append();
		(*row)[serverColumns.name] = "Status";
		(*row)[serverColumns.pointer] = this;
		serverRows.emplace(this, row);
	}

	void MainBox::focusEntry() {
		chatEntry.grab_focus();
	}

	void MainBox::addServer(PingPong::Server *server, bool focus) {
		if (serverRows.count(server) != 0)
			return;
		auto row = serverModel->append();
		(*row)[serverColumns.name] = server->id;
		(*row)[serverColumns.pointer] = server;
		serverRows.emplace(server, row);
		if (focus)
			focusView(server);
	}

	void MainBox::addChannel(PingPong::Channel *channel, bool focus) {
		if (serverRows.count(channel->server) == 0)
			addServer(channel->server);
		auto iter = serverRows.at(channel->server);
		auto row = serverModel->append(iter->children());
		serverTree.expand_row(Gtk::TreeModel::Path(iter), false);
		(*row)[serverColumns.name] = channel->name;
		(*row)[serverColumns.pointer] = channel;
		channelRows.emplace(channel, row);
		if (focus)
			focusView(channel);
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

	LineView & MainBox::getLineView(void *ptr) {
		if (views.count(ptr) == 0)
			return views.emplace(ptr, LineView()).first->second;
		return views.at(ptr);
	}

	void MainBox::setTopic(void *ptr, const std::string &topic) {
		topics[ptr] = topic;
		if (activeView == ptr)
			topicLabel.set_text(topic);
	}

	void MainBox::focusView(void *ptr) {
		if (topics.count(ptr) != 0)
			topicLabel.set_text(topics.at(ptr));
		else
			topicLabel.set_text("");
		activeView = ptr;
		scrolled.set_child(getLineView(ptr));
		if (serverRows.count(ptr) != 0)
			serverTree.get_selection()->select(serverRows.at(ptr));
		else if (channelRows.count(reinterpret_cast<PingPong::Channel *>(ptr)) != 0) {
			PingPong::Channel *channel = reinterpret_cast<PingPong::Channel *>(ptr);
			topicLabel.set_text(topics[ptr] = std::string(channel->topic));
			serverTree.get_selection()->select(channelRows.at(channel));
		}
	}

	void MainBox::serverRowActivated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *) {
		if (auto iter = serverModel->get_iter(path))
			focusView((*iter)[serverColumns.pointer]);
	}
}
