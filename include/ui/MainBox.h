#pragma once

#include <gtkmm.h>

#include "Connection.h"
#include "ui/BasicEntry.h"

namespace Reden {
	class MainBox: public Gtk::Box {
		public:
			MainBox() = delete;
			MainBox(ConnectionMap &);

		private:
			ConnectionMap &connections;
			Gtk::TreeView serverTree, userTree;
			Glib::RefPtr<Gtk::ListStore> serverModel, userModel;
			Gtk::Separator leftSeparator, rightSeparator;
			Gtk::Box chatBox {Gtk::Orientation::VERTICAL};
			Gtk::Label topic;
			Gtk::ScrolledWindow scrolled;
			Gtk::Grid chatGrid;
			BasicEntry chatEntry;
	};
}
