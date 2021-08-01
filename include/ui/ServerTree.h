#pragma once

#include <gtkmm.h>
#include <unordered_map>

namespace PingPong {
	class Server;
	class Channel;
	class User;
}

namespace Reden {
	class ServerTree: public Gtk::TreeView {
		public:
			// I'm using a void pointer here because serverRows can store both PingPong::Server pointers and also a
			// dummy pointer to this MainBox for the status window.
			std::unordered_map<void *, Gtk::TreeModel::iterator> serverRows;
			std::unordered_map<PingPong::Channel *, Gtk::TreeModel::iterator> channelRows;
			std::unordered_map<PingPong::User *, Gtk::TreeModel::iterator> userRows;

			ServerTree();

			void addStatusRow();
			void add(PingPong::Server *, bool focus);
			void add(PingPong::Channel *, bool focus);
			void add(PingPong::User *, bool focus);
			void erase(PingPong::Server *, void *active_view);
			void erase(PingPong::Channel *, void *active_view);
			void erase(PingPong::User *, void *active_view);
			void cursorChanged();
			Gtk::TreeModel::Path getPath(const Gtk::TreeModel::const_iterator &);

			sigc::signal<void()> signal_status_focus_requested();
			sigc::signal<void(PingPong::Channel *)> signal_channel_focus_requested();
			sigc::signal<void(PingPong::Server *)>   signal_server_focus_requested();
			sigc::signal<void(PingPong::User *)>       signal_user_focus_requested();
			sigc::signal<void(void *)> signal_focus_requested();
			sigc::signal<void(void *)> signal_erase_requested();

		private:
			struct Columns: public Gtk::TreeModelColumnRecord {
				Columns() {
					add(name);
					add(pointer);
				}

				Gtk::TreeModelColumn<Glib::ustring> name;
				Gtk::TreeModelColumn<void *> pointer;
			};

			Glib::RefPtr<Gtk::TreeStore> model;
			Columns columns;

			void focusView(PingPong::Channel *);
			void focusView(PingPong::Server *);
			void focusView(PingPong::User *);
			void focusVoid(void *);
			void rowActivated(const Gtk::TreeModel::Path &, Gtk::TreeViewColumn *);
			sigc::signal<void()> signal_status_focus_requested_;
			sigc::signal<void(PingPong::Channel *)> signal_channel_focus_requested_;
			sigc::signal<void(PingPong::Server *)>   signal_server_focus_requested_;
			sigc::signal<void(PingPong::User *)>       signal_user_focus_requested_;
			sigc::signal<void(void *)> signal_focus_requested_;
			sigc::signal<void(void *)> signal_erase_requested_;
	};
}
