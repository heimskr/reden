#pragma once

#include <gtkmm.h>

#include "Connection.h"
#include "ui/BasicEntry.h"

namespace PingPong {
	class Server;
	class Channel;
}

namespace Reden {
	class RedenWindow;

	class MainBox: public Gtk::Box {
		public:
			MainBox() = delete;
			MainBox(RedenWindow &);

			void addServer(PingPong::Server *);
			void addChannel(PingPong::Channel *);

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
			std::unordered_map<PingPong::Server *, Gtk::TreeModel::iterator> serverRows;
			std::unordered_map<PingPong::Channel *, Gtk::TreeModel::iterator> channelRows;
	};
}
