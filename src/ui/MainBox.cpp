#include "ui/MainBox.h"
#include "ui/RedenWindow.h"

#include "pingpong/core/Debug.h"
#include "pingpong/core/Server.h"

namespace Reden {
	MainBox::MainBox(RedenWindow &parent_): Gtk::Box(Gtk::Orientation::HORIZONTAL), parent(parent_) {
		serverTree.set_vexpand(true);
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
}
