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
		appendColumn(serverTree, "Name", serverColumns.name);
		chatBox.set_expand(true);
		userTree.set_vexpand(true);
		serverTree.set_size_request(300, -1);
		userTree.set_size_request(300, -1);
		serverTree.set_can_focus(false);
		userTree.set_can_focus(false);
		append(serverTree);
		append(leftSeparator);
		append(chatBox);
		append(rightSeparator);
		append(userTree);
		chatBox.append(topic);
		chatBox.append(scrolled);
		chatBox.append(chatEntry);
		chatEntry.add_css_class("unrounded");
		scrolled.set_vexpand(true);
		scrolled.set_child(chatGrid);
		chatEntry.signal_activate().connect([this]() {
			if (parent.irc->activeServer) {
				parent.irc->activeServer->quote(chatEntry.get_text());
			} else {
				DBG("No active server.");
			}

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

	void MainBox::addServer(PingPong::Server *server) {
		if (serverRows.count(server) != 0)
			return;
		auto row = serverModel->append();
		(*row)[serverColumns.name] = server->id;
		(*row)[serverColumns.pointer] = server;
		serverRows.emplace(server, row);
	}

	void MainBox::addChannel(PingPong::Channel *channel) {
		if (serverRows.count(channel->server) == 0)
			addServer(channel->server);
		auto iter = serverRows.at(channel->server);
		auto row = serverModel->append(iter->children());
		serverTree.expand_row(Gtk::TreeModel::Path(iter), false);
		(*row)[serverColumns.name] = channel->name;
		(*row)[serverColumns.pointer] = channel;
		channelRows.emplace(channel, row);
	}

	void MainBox::eraseServer(PingPong::Server *server) {
		if (serverRows.count(server) == 0)
			return;
		auto iter = serverRows.at(server);
		serverModel->erase(iter);
		serverRows.erase(server);
	}

	void MainBox::eraseChannel(PingPong::Channel *channel) {
		if (channelRows.count(channel) == 0)
			return;
		auto iter = channelRows.at(channel);
		serverModel->erase(iter);
		channelRows.erase(channel);
	}

	LineView & MainBox::getLineView(void *ptr) {
		if (views.count(ptr) == 0)
			views.emplace(ptr, LineView());
		return views.at(ptr);
	}

	void MainBox::focusServer(void *ptr) {
		scrolled.set_child(getLineView(ptr));
	}

	void MainBox::serverRowActivated(const Gtk::TreeModel::Path &path, Gtk::TreeViewColumn *) {
		if (auto iter = serverModel->get_iter(path))
			focusServer((*iter)[serverColumns.pointer]);
	}
}
