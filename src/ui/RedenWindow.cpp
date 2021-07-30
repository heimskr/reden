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
#include "pingpong/events/Join.h"
#include "pingpong/events/Mode.h"
#include "pingpong/events/NamesUpdated.h"
#include "pingpong/events/Part.h"
#include "pingpong/events/Privmsg.h"
#include "pingpong/events/Raw.h"
#include "pingpong/events/ServerStatus.h"
#include "pingpong/events/Topic.h"
#include "lib/formicine/futil.h"

namespace Reden {
	RedenWindow::RedenWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder_):
	Gtk::ApplicationWindow(cobject), client(*this), builder(builder_), mainBox(*this) {
		irc = std::make_shared<PingPong::IRC>();

		header = builder->get_widget<Gtk::HeaderBar>("headerbar");
		set_titlebar(*header);

		cssProvider = Gtk::CssProvider::create();
		if (FS::fileExists("custom.css")) {
			try {
				cssProvider->load_from_data(App::getText("/com/heimskr/reden/style.css") + std::string("\n")
					+ FS::readFile("custom.css"));
			} catch (const std::exception &) {
				cssProvider->load_from_resource("/com/heimskr/reden/style.css");
			}
		} else
			cssProvider->load_from_resource("/com/heimskr/reden/style.css");
		Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), cssProvider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

		irc->init();

		addListeners();

		add_action("connect", Gio::ActionMap::ActivateSlot([this] {
			auto *connect = new ConnectDialog("Connect", *this, true);
			dialog.reset(connect);
			connect->signal_submit().connect(
			[this](const Glib::ustring &hostname, const Glib::ustring &port_str, const Glib::ustring &nick) {
				long port;
				if (!formicine::util::parse_long(port_str, port)) {
					delay([this] { error("Invalid port."); });
					return;
				}
				irc->connect(hostname, nick, port, false);
				mainBox.focusEntry();
			});
			connect->show();
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

		set_child(mainBox);
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

	Glib::ustring RedenWindow::getInput() const {
		return mainBox.getInput();
	}

	void RedenWindow::setInput(const Glib::ustring &text) {
		mainBox.setInput(text);
	}

	int RedenWindow::getCursor() const {
		return mainBox.getCursor();
	}

	void RedenWindow::setCursor(int cursor) {
		mainBox.setCursor(cursor);
	}

	void RedenWindow::addListeners() {
		PingPong::Events::listen<PingPong::JoinEvent>([this](PingPong::JoinEvent *ev) {
			const bool self = ev->who->isSelf();
			auto channel = ev->channel;
			auto name = ev->who->name;
			queue([this, channel, name, self] {
				if (self)
					mainBox.addChannel(channel.get(), true);
				mainBox[channel].joined(name, channel->name);
				mainBox.updateChannel(*channel);
			});
		});

		PingPong::Events::listen<PingPong::ModeEvent>([this](PingPong::ModeEvent *ev) {
			auto who = ev->who;
			auto modeset = ev->modeSet;
			if (auto channel = ev->getChannel(ev->server))
				queue([this, channel, who, modeset] {
					mainBox[channel].mode(channel, who, modeset);
					mainBox.updateChannel(*channel);
				});
		});

		PingPong::Events::listen<PingPong::NamesUpdatedEvent>([this](PingPong::NamesUpdatedEvent *ev) {
			auto channel = ev->channel;
			queue([this, channel] {
				mainBox.updateChannel(*channel);
			});
		});

		PingPong::Events::listen<PingPong::PartEvent>([this](PingPong::PartEvent *ev) {
			if (ev->who->isSelf()) {
				auto channel = ev->channel;
				queue([this, channel] {
					mainBox.eraseChannel(channel.get());
				});
			}
		});

		PingPong::Events::listen<PingPong::PrivmsgEvent>([this](PingPong::PrivmsgEvent *ev) {
			if (ev->isChannel()) {
				const std::string content = ev->content;
				auto channel = ev->server->getChannel(ev->where);
				const std::string name = channel->withHat(ev->speaker);
				queue([this, content, channel, name] {
					mainBox[channel].addMessage(name, content);
				});
			}
		});

		PingPong::Events::listen<PingPong::RawInEvent>([this](PingPong::RawInEvent *ev) {
			auto server = ev->server;
			auto raw = ev->rawIn;
			while (!raw.empty() && (raw.back() == '\r' || raw.back() == '\n'))
				raw.pop_back();
			queue([this, server, raw] {
				mainBox.addServer(server, false);
				mainBox[server] += "<< " + raw;
			});
		});

		PingPong::Events::listen<PingPong::RawOutEvent>([this](PingPong::RawOutEvent *ev) {
			auto server = ev->server;
			auto raw = ev->rawOut;
			while (!raw.empty() && (raw.back() == '\r' || raw.back() == '\n'))
				raw.pop_back();
			queue([this, server, raw] {
				mainBox.addServer(server, false);
				mainBox[server] += ">> " + raw;
			});
		});

		PingPong::Events::listen<PingPong::ServerStatusEvent>([this](PingPong::ServerStatusEvent *ev) {
			switch (ev->server->getStatus()) {
				case PingPong::Server::Stage::Ready: {
					auto server = ev->server;
					queue([this, server] {
						mainBox.addServer(server, true);
						mainBox.addStatus("Connected to " + server->id + " (" + server->hostname + ":"
							+ std::to_string(server->port) + ")");
					});
					break;
				}
				case PingPong::Server::Stage::Dead: {
					auto server = ev->server;
					queue([this, server] {
						mainBox.eraseServer(server);
					});
					break;
				}
				default:
					break;
			}
		});

		PingPong::Events::listen<PingPong::TopicEvent>([this](PingPong::TopicEvent *ev) {
			auto channel = ev->channel;
			auto who = ev->who;
			queue([this, channel, who] {
				mainBox.setTopic(channel.get(), std::string(channel->topic));
				mainBox[channel].topicChanged(channel, who, std::string(channel->topic));
			});
		});
	}
}
