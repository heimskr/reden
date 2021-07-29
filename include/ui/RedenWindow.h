#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>
#include <mutex>

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

			/** Causes a function to occur on the next Gtk tick (or possibly later). Not thread-safe. */
			void delay(std::function<void()>);
			/** Queues a function to be executed in the Gtk thread. Thread-safe. Can be used from any thread. */
			void queue(std::function<void()>);
			void alert(const Glib::ustring &message, Gtk::MessageType = Gtk::MessageType::INFO, bool modal = true,
					bool use_markup = false);
			void error(const Glib::ustring &message, bool modal = true, bool use_markup = false);

		private:
			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			ConnectionMap connections;
			MainBox mainBox;
			std::unique_ptr<Gtk::Dialog> dialog;
			std::list<std::function<void()>> functionQueue;
			std::mutex functionQueueMutex;
			Glib::Dispatcher functionQueueDispatcher;

			void addListeners();
	};
}
