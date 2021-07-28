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

	void MainBox::addServer(const std::string &server_name) {
		if (serverRows.count(server_name) != 0)
			return;
		std::cout << "Adding server_name[" << server_name << "]\n";
		auto row = serverModel->append();
		(*row)[serverColumns.name] = server_name;
		serverRows.emplace(server_name, row);
		for (int i = 1; i <= 5; ++i) {
			auto subrow = serverModel->append(row->children());
			(*subrow)[serverColumns.name] = std::to_string(i);
		}
	}
}
