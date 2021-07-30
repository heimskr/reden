#include "core/Client.h"
#include "ui/RedenWindow.h"

#include "pingpong/commands/Join.h"
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
	}
}
