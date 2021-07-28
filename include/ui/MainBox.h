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

			void addServer(const std::string &server_name);

		private:
			struct ServerColumns: public Gtk::TreeModelColumnRecord {
				ServerColumns() {
					add(name);
				}

				Gtk::TreeModelColumn<Glib::ustring> name;
			};

			RedenWindow &parent;
			Gtk::TreeView serverTree, userTree;
			Glib::RefPtr<Gtk::TreeStore> serverModel, userModel;
			Gtk::Separator leftSeparator, rightSeparator;
			Gtk::Box chatBox {Gtk::Orientation::VERTICAL};
			Gtk::Label topic;
			Gtk::ScrolledWindow scrolled;
			Gtk::Grid chatGrid;
			BasicEntry chatEntry;
			ServerColumns serverColumns;
			std::unordered_map<std::string, Gtk::TreeModel::iterator> serverRows;
	};
}
