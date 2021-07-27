#include "ui/MainBox.h"

namespace Reden {
	MainBox::MainBox(ConnectionMap &connections_): Gtk::Box(Gtk::Orientation::HORIZONTAL), connections(connections_) {
		serverTree.set_vexpand(true);
		chatBox.set_expand(true);
		userTree.set_vexpand(true);
		serverTree.set_size_request(300, -1);
		userTree.set_size_request(300, -1);
		append(serverTree);
		append(chatBox);
		append(userTree);
		chatBox.append(topic);
		chatBox.append(scrolled);
		chatBox.append(chatEntry);
		scrolled.set_vexpand(true);
		scrolled.set_child(chatGrid);	
	}
}
