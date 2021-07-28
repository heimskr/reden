#pragma once

#include <gtkmm.h>

#include <map>
#include <unordered_map>

#include "Connection.h"
#include "ui/BasicEntry.h"
#include "ui/LineView.h"

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

			void focusEntry();
			void addServer(PingPong::Server *);
			void addChannel(PingPong::Channel *);
			void eraseServer(PingPong::Server *);
			void eraseChannel(PingPong::Channel *);
			LineView & getLineView(void *ptr);

		private:
			struct ServerColumns: public Gtk::TreeModelColumnRecord {
				ServerColumns() {
					add(name);
					add(pointer);
				}

				Gtk::TreeModelColumn<Glib::ustring> name;
				Gtk::TreeModelColumn<void *> pointer;
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
			// I'm using a void pointer here because serverRows can store both PingPong::Server pointers and also a
			// dummy pointer to this MainBox for the status window.
			std::unordered_map<void *, Gtk::TreeModel::iterator> serverRows;
			std::unordered_map<PingPong::Channel *, Gtk::TreeModel::iterator> channelRows;
			std::unordered_map<void *, LineView> views;

			void addStatusRow();
			void focusServer(void *);
			void serverRowActivated(const Gtk::TreeModel::Path &, Gtk::TreeViewColumn *);
	};
}
