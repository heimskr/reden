#pragma once

#include <gtkmm.h>
#include <map>

#include "ui/BasicEntry.h"

namespace Reden {
	class Client;

	class ConnectDialog: public Gtk::Dialog {
		public:
			ConnectDialog(const Glib::ustring &title, Gtk::Window &parent, Client &, bool modal = true);

			sigc::signal<void(const Glib::ustring &host, const Glib::ustring &port, const Glib::ustring &nick,
			                  const Glib::ustring &password)>
			signal_submit() const { return signal_submit_; }

		private:
			struct ServerConfig {
				Glib::ustring host, nick, password;
				int port = 6667;
				ServerConfig() = default;
				ServerConfig(const Glib::ustring &host_, const Glib::ustring &nick_, const Glib::ustring &password_,
				             int port_ = 6667):
					host(host_), nick(nick_), password(password_), port(port_) {}
				ServerConfig(const Glib::ustring &combined);
				operator Glib::ustring() const;
			};

			struct Columns: Gtk::TreeModel::ColumnRecord {
				Columns() {
					add(name);
					add(config);
				}

				Gtk::TreeModelColumn<Glib::ustring> name;
				Gtk::TreeModelColumn<std::shared_ptr<ServerConfig>> config;
			};

			Client &client;
			BasicEntry hostEntry, portEntry, nickEntry, usernameEntry, passwordEntry;
			Gtk::Box buttonBox {Gtk::Orientation::HORIZONTAL};
			Gtk::Button cancelButton {"Cancel"}, okButton {"OK"};
			Gtk::ComboBox serverCombo;
			Glib::RefPtr<Gtk::ListStore> comboModel;
			std::map<Glib::ustring, std::shared_ptr<ServerConfig>> serverConfigs;
			Columns columns;
			std::unique_ptr<Gtk::Dialog> subdialog;
			sigc::signal<void(const Glib::ustring &host, const Glib::ustring &port, const Glib::ustring &nick,
			                  const Glib::ustring &password)> signal_submit_;

			void on_combo_changed();
			void submit();
			void populateConfigs();
			void resetCombo();
			void fill(const ServerConfig &);
	};
}
