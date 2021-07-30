#include "core/Client.h"

#include "pingpong/commands/Join.h"
#include "pingpong/core/Server.h"

namespace Reden {
	void Client::addCommands() {
		add("join", 1,  1, true, [&](PingPong::Server *server, const InputLine &il) {
			const Glib::ustring &first = il.first();
			std::cout << "Join: " << server;
			std::cout << " (" << server->id << ")\n";
			waitForServer(server, PingPong::Server::Stage::Ready, [=]() {
				std::cout << "Sending JoinCommand.\n";
				PingPong::JoinCommand(server, first).send();
			});
		});
	}
}
