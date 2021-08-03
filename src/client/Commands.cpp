#include "core/Client.h"
#include "core/TabCompletion.h"
#include "ui/RedenWindow.h"

#include "pingpong/commands/Join.h"
#include "pingpong/commands/Nick.h"
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
			waitForServer(server, PingPong::Server::Stage::Ready, [=] {
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

		add("msg", 2, -1, true, [this](PingPong::Server *server, const InputLine &il) {
			PingPong::PrivmsgCommand(server, il.first(), il.rest()).send();
		}, &completePlain);

		add("nick", 0, 1, true, [this](PingPong::Server *server, const InputLine &il) {
			if (il.args.size() == 0)
				window.box.active().add("Current nick: " + server->getNick());
			else
				PingPong::NickCommand(server, il.first()).send();
		});

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

		add("quote", 1, -1, true, [this](PingPong::Server *server, const InputLine &il) {
			server->quote(il.body);
		});

		add("set",   0, -1, false, [this](PingPong::Server *, const InputLine &il) {
			config.readIfEmpty(DEFAULT_CONFIG_DB);

			auto with_defaults = config.withDefaults();

			if (il.args.empty()) {
				for (const auto &[group, submap]: with_defaults) {
					window.box.addStatus("[" + group + "]", false);
					for (const auto &[key, value]: submap)
						window.box.addStatus("    " + key + " = " + ConfigDB::escape(value), false);
				}

				return;
			}

			const std::string &first = il.first();

			std::pair<std::string, std::string> parsed;

			if (first.find('.') == std::string::npos) {
				parsed.second = first;
				for (const auto &gpair: with_defaults)
					if (gpair.second.count(first) == 1) {
						if (!parsed.first.empty()) {
							window.box.status().error("Multiple groups contain the key <span weight=\"bold\">" +
								Glib::Markup::escape_text(first) + "</span>.", true);
							return;
						}

						parsed.first = gpair.first;
					}
			} else
				try {
					parsed = ConfigDB::parsePair(first);
				} catch (const std::invalid_argument &) {
					window.box.status().error("Couldn't parse setting " + ansi::bold(first));
					return;
				}

			if (il.args.size() == 1) {
				try {
					const Reden::Value &value = config.getPair(parsed);
					window.box.addStatus(parsed.first + "." + parsed.second + " = " + ConfigDB::escape(value), false);
				} catch (const std::out_of_range &err) {
					std::cerr << "std::out_of_range: " << err.what() << "\n";
					window.box.status().error("No configuration option for " + first + ".");
				}
			} else {
				const std::string joined = formicine::util::join(il.args.begin() + 1, il.args.end());

				// Special case: setting a value to "-" removes it from the database.
				if (joined == "-") {
					if (config.remove(parsed.first, parsed.second, true, true))
						window.box.addStatus("Removed " + parsed.first + "." + parsed.second + ".");
					else
						window.box.status().error("Couldn't find " + parsed.first + "." + parsed.second + ".");
				} else {
					const ValueType type = ConfigDB::getValueType(joined);
					if (type == ValueType::Invalid)
						config.insert(parsed.first, parsed.second, {joined});
					else
						config.insertAny(parsed.first, parsed.second, joined);
					window.box.addStatus("Set " + parsed.first + "." + parsed.second + " to " + joined + ".");
				}
			}
		}, &completeSet);
	}
}
