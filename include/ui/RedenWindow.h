#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>

#include "Connection.h"
#include "ui/MainBox.h"

#include "pingpong/core/IRC.h"

namespace Reden {
	class RedenWindow: public Gtk::ApplicationWindow {
		public:
			PingPong::IRC *irc = nullptr;
			Gtk::HeaderBar *header;

			RedenWindow(BaseObjectType *, const Glib::RefPtr<Gtk::Builder> &);

			static RedenWindow * create();

			void delay(std::function<void()>);
			void alert(const Glib::ustring &message, Gtk::MessageType = Gtk::MessageType::INFO, bool modal = true,
					bool use_markup = false);
			void error(const Glib::ustring &message, bool modal = true, bool use_markup = false);

		private:
			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			ConnectionMap connections;
			MainBox mainBox;
			std::unique_ptr<Gtk::Dialog> dialog;
	};
}
