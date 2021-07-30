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
}
