#include "core/Client.h"
#include "ui/RedenWindow.h"

namespace Reden {
	std::shared_ptr<PingPong::IRC> Client::irc() const {
		return window.irc;
	}

	void Client::completeMessage(Glib::ustring &, size_t cursor, ssize_t word_offset) {
		(void) cursor;
		(void) word_offset;
	}

	InputLine Client::getInputLine(const Glib::ustring &str) const {
		// if (!str.empty() && ui.activeWindow == ui.statusWindow && str.front() != '/')
		// 	return InputLine("/" + str);
		return InputLine(str);
	}
}
