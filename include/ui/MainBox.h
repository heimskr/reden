#pragma once

#include <gtkmm.h>

#include "Connection.h"
#include "ui/BasicEntry.h"

namespace Reden {
	class RedenWindow;

	class MainBox: public Gtk::Box {
		public:
			MainBox() = delete;
			MainBox(RedenWindow &);

		private:
			RedenWindow &parent;
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
