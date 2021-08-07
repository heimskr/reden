#include <iostream>

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "App.h"
#include "FS.h"
#include "ui/ConnectDialog.h"
#include "ui/RedenWindow.h"

#include "pingpong/core/IRC.h"
#include "pingpong/net/NetError.h"
#include "lib/formicine/futil.h"

namespace Reden {
	RedenWindow::RedenWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder_):
	Gtk::ApplicationWindow(cobject), client(*this), box(*this), builder(builder_) {
		irc = std::make_shared<PingPong::IRC>();

		header = builder->get_widget<Gtk::HeaderBar>("headerbar");
		set_titlebar(*header);

		cssProvider = Gtk::CssProvider::create();
		loadCSS();
		Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), cssProvider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

		irc->init();

		client.init();

		add_action("connect", Gio::ActionMap::ActivateSlot([this] {
			auto *connect = new ConnectDialog("Connect", *this, client, true);
			dialog.reset(connect);
			connect->signal_submit().connect(
			[this](const Glib::ustring &hostname, const Glib::ustring &port_str, const Glib::ustring &nick,
			       const Glib::ustring &password) {
				long port;
				if (!formicine::util::parse_long(port_str, port)) {
					delay([this] { error("Invalid port."); });
					return;
				}
				try {
					irc->connect(hostname, nick, port, false, password);
					box.focusEntry();
				} catch (std::exception &err) {
					Glib::ustring what = err.what();
					delay([=, this] {
						error("Error while connecting to " + hostname + ":" + std::to_string(port) + ": " + what);
					});
				}
			});
			connect->show();
		}));

		add_action("reload-css", Gio::ActionMap::ActivateSlot([this] {
			loadCSS();
		}));

		functionQueueDispatcher.connect([this] {
			auto lock = std::unique_lock(functionQueueMutex);
			for (auto fn: functionQueue)
				fn();
			functionQueue.clear();
		});

		signal_hide().connect([this] {
			if (irc) {
				for (auto &[name, server]: irc->servers)
					server->quit("Leaving");
				// Hack: wait for servers to die
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				irc.reset();
			}
		});

		set_child(box);
	}

	RedenWindow * RedenWindow::create() {
		auto builder = Gtk::Builder::create_from_resource("/com/heimskr/reden/window.ui");
		auto window = Gtk::Builder::get_widget_derived<RedenWindow>(builder, "reden_window");
		if (!window)
			throw std::runtime_error("No \"reden_window\" object in window.ui");
		return window;
	}

	void RedenWindow::delay(std::function<void()> fn) {
		add_tick_callback([fn](const auto &) {
			fn();
			return false;
		});
	}

	void RedenWindow::queue(std::function<void()> fn) {
		{
			auto lock = std::unique_lock(functionQueueMutex);
			functionQueue.push_back(fn);
		}
		functionQueueDispatcher.emit();
	}

	void RedenWindow::alert(const Glib::ustring &message, Gtk::MessageType type, bool modal, bool use_markup) {
		dialog.reset(new Gtk::MessageDialog(*this, message, use_markup, type, Gtk::ButtonsType::OK, modal));
		dialog->signal_response().connect([this](int) {
			dialog->close();
		});
		dialog->show();
	}

	void RedenWindow::error(const Glib::ustring &message, bool modal, bool use_markup) {
		alert(message, Gtk::MessageType::ERROR, modal, use_markup);
	}

	void RedenWindow::loadCSS(const std::string &css) {
		if (FS::fileExists("custom.css")) {
			try {
				cssProvider->load_from_data(App::getText("/com/heimskr/reden/style.css") + std::string("\n") + css
					+ "\n" + FS::readFile("custom.css"));
				return;
			} catch (const std::exception &) {}
		}

		if (!css.empty()) {
			cssProvider->load_from_data(App::getText("/com/heimskr/reden/style.css") + std::string("\n") + css);
		} else
			cssProvider->load_from_resource("/com/heimskr/reden/style.css");
	}
}
