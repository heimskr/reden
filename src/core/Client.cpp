#include "core/Client.h"
#include "core/Util.h"
#include "ui/RedenWindow.h"

#include "pingpong/core/Server.h"

namespace Reden {
	std::shared_ptr<PingPong::IRC> Client::irc() const {
		return window.irc;
	}

	std::vector<Glib::ustring> Client::commandMatches(const Glib::ustring &command_name) {
		std::vector<Glib::ustring> matches;

		const size_t command_length = command_name.length();

		for (const std::pair<Glib::ustring, Command> &pair: commandHandlers) {
			const Glib::ustring &candidate_name = pair.first;
			if (candidate_name.substr(0, command_length) == command_name)
				matches.push_back(candidate_name);
		}

		return matches;
	}

	InputLine Client::getInputLine(const Glib::ustring &str) const {
		// if (!str.empty() && ui.activeWindow == ui.statusWindow && str.front() != '/')
		// 	return InputLine("/" + str);
		return InputLine(str);
	}

	bool Client::handleLine(const InputLine &il) {
		const int nargs = static_cast<int>(il.args.size());
		const std::string &name = il.command;

		auto range = commandHandlers.equal_range(name);
		// Quit if there are no matching handlers.
		if (range.first == range.second)
			return false;

		for (auto it = range.first; it != range.second; ++it) {
			auto &[min, max, needs_serv, fn, comp_fn, suggestion_fns] = it->second;
			if (max == 0 && nargs != 0)
				window.box.log("/" + name + " doesn't accept any arguments.");
			else if (min == max && nargs != min)
				window.box.log("/" + name + " expects " + std::to_string(min) + " argument" + (min == 1? "." : "s."));
			else if (nargs < min)
				window.box.log("/" + name + " expects at least " + std::to_string(min) + " argument"
					+ (min == 1? "." : "s."));
			else if (max != -1 && max < nargs)
				window.box.log("/" + name + " expects at most " + std::to_string(max) + " argument"
					+ (min == 1? "." : "s."));
			else {
				PingPong::Server *active_server = window.box.activeServer();
				if (needs_serv && !active_server)
					window.box.log("No server is selected.");
				else if (fn(active_server, il))
					break;
			}
		}

		return true;
	}
}
