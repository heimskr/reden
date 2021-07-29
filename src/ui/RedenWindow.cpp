#include <iostream>

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "ui/ConnectDialog.h"
#include "ui/RedenWindow.h"
#include "pingpong/events/Join.h"
#include "pingpong/events/Part.h"
#include "pingpong/events/Privmsg.h"
#include "pingpong/events/ServerStatus.h"
#include "pingpong/events/Topic.h"
#include "lib/formicine/futil.h"

namespace Reden {
	RedenWindow::RedenWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder_):
	Gtk::ApplicationWindow(cobject), builder(builder_), mainBox(*this) {
		irc = new PingPong::IRC;

		header = builder->get_widget<Gtk::HeaderBar>("headerbar");
		set_titlebar(*header);

		cssProvider = Gtk::CssProvider::create();
		cssProvider->load_from_resource("/com/heimskr/reden/style.css");
		Gtk::StyleContext::add_provider_for_display(Gdk::Display::get_default(), cssProvider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

		irc->init();

		PingPong::Events::listen<PingPong::JoinEvent>([this](PingPong::JoinEvent *ev) {
			if (ev->who->isSelf()) {
				auto channel = ev->channel;
				auto name = ev->who->name;
				queue([this, channel, name] {
					mainBox.addChannel(channel.get(), true);
					mainBox.getLineView(channel.get()).joined(name, channel->name);
				});
			}
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
				std::string name;
				name += static_cast<char>(channel->getHats(ev->speaker).highest());
				name += ev->speaker->name;
				queue([this, content, channel, name] {
					mainBox.getLineView(channel.get()).addMessage(name, content);
				});
			}
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
			queue([this, channel] {
				mainBox.setTopic(channel.get(), std::string(channel->topic));
			});
		});

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
				delete irc;
				irc = nullptr;
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
}
