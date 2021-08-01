#include "core/Client.h"
#include "core/TabCompletion.h"
#include "ui/RedenWindow.h"

#include "pingpong/commands/Join.h"
#include "pingpong/commands/Privmsg.h"
#include "pingpong/core/Server.h"

namespace Reden {
	void Client::addCommands() {
		add("clear", 0, 0, false, [this](PingPong::Server *, const InputLine &) {
			window.box.active().clear();
		});

		add("join", 1,  1, true, [&](PingPong::Server *server, const InputLine &il) {
			const Glib::ustring &first = il.first();
			waitForServer(server, PingPong::Server::Stage::Ready, [=]() {
				PingPong::JoinCommand(server, first).send();
			});
		});

		add("me", 1, -1, true, [&](PingPong::Server *, const InputLine &il) {
			const auto &active = window.box.active();
			if (!active.isAlive()) {
				return;
			}

			const Glib::ustring message = "\1ACTION " + il.body + "\1";
			if (active.isChannel())
				PingPong::PrivmsgCommand(active.getChannel(), message).send();
			else if (active.isUser())
				PingPong::PrivmsgCommand(active.getUser(), message).send();
		}, &completePlain);
	}
}
