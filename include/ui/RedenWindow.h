#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>
#include <mutex>

#include "core/Client.h"
#include "ui/MainBox.h"

namespace PingPong {
	class IRC;
}

namespace Reden {
	class RedenWindow: public Gtk::ApplicationWindow {
		public:
			std::shared_ptr<PingPong::IRC> irc;
			Client client;
			Gtk::HeaderBar *header;
			MainBox box;

			RedenWindow(BaseObjectType *, const Glib::RefPtr<Gtk::Builder> &);

			static RedenWindow * create();

			/** Causes a function to occur on the next Gtk tick (or possibly later). Not thread-safe. */
			void delay(std::function<void()>);
			/** Queues a function to be executed in the Gtk thread. Thread-safe. Can be used from any thread. */
			void queue(std::function<void()>);
			void alert(const Glib::ustring &message, Gtk::MessageType = Gtk::MessageType::INFO, bool modal = true,
					bool use_markup = false);
			void error(const Glib::ustring &message, bool modal = true, bool use_markup = false);

			/** Loads the built-in CSS, then the provided string, then custom.css if it exists. */
			void loadCSS(const std::string & = "");

		private:
			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			std::unique_ptr<Gtk::Dialog> dialog;
			std::list<std::function<void()>> functionQueue;
			std::mutex functionQueueMutex;
			Glib::Dispatcher functionQueueDispatcher;

			void addListeners();
	};
}
