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
	}

	void MainBox::addServer(PingPong::Server *server) {
		if (serverRows.count(server) != 0)
			return;
		std::cout << "Adding server[" << server->id << "]\n";
		auto row = serverModel->append();
		(*row)[serverColumns.name] = server->id;
		serverRows.emplace(server, row);
	}

	void MainBox::addChannel(PingPong::Channel *channel) {
		if (serverRows.count(channel->server) == 0)
			addServer(channel->server);
		auto iter = serverRows.at(channel->server);
		auto row = serverModel->append(iter->children());
		(*row)[serverColumns.name] = channel->name;
		channelRows.emplace(channel, row);
	}
}
