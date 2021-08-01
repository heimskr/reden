#include "core/Client.h"
#include "core/TabCompletion.h"
#include "ui/RedenWindow.h"

#include "pingpong/commands/Join.h"
#include "pingpong/commands/Part.h"
#include "pingpong/commands/Privmsg.h"
#include "pingpong/core/Server.h"

namespace Reden {
	void Client::addCommands() {
		add("clear", 0, 0, false, [this](PingPong::Server *, const InputLine &) {
			window.box.active().clear();
		});

		add("close", 0, 0, false, [this](PingPong::Server *, const InputLine &) {
			if (PingPong::User *user = window.box.activeUser())
				window.box.erase(user);
			else
				window.box.active().error("Can't close window.", false);
		});

		add("join", 1,  1, true, [this](PingPong::Server *server, const InputLine &il) {
			const Glib::ustring &first = il.first();
			waitForServer(server, PingPong::Server::Stage::Ready, [=]() {
				PingPong::JoinCommand(server, first).send();
			});
		});

		add("me", 1, -1, true, [this](PingPong::Server *, const InputLine &il) {
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

		add("part", 0, -1, true, [this](PingPong::Server *server, const InputLine &il) {
			PingPong::Channel *active_channel = nullptr;

			const auto &active = window.box.active();
			if (active.isAlive())
				active_channel = window.box.activeChannel();

			if ((il.args.empty() || il.first()[0] != '#') && !active_channel)
				noChannel();
			else if (il.args.empty())
				PingPong::PartCommand(active_channel).send();
			else if (il.first()[0] != '#')
				PingPong::PartCommand(active_channel, il.body).send();
			else if (std::shared_ptr<PingPong::Channel> cptr = server->getChannel(il.first()))
				PingPong::PartCommand(cptr, il.rest()).send();
			else
				noChannel(il.first());
		});
	}
}
