#pragma once

#include <gtkmm.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "config/Cache.h"
#include "core/TabCompletion.h"

namespace PingPong {
	class IRC;
}

namespace Reden {
	class RedenWindow;

	class Client {
		public:
			std::unordered_map<std::string, CompletionState> completionStates;
			ConfigCache cache;

			Client() = delete;
			Client(const Client &) = delete;
			Client(RedenWindow &window_): window(window_) {}

			Client & operator=(const Client &) = delete;

			std::vector<Glib::ustring> commandMatches(const Glib::ustring &);

			/** Completes a message for a given cursor position. The word_offset parameter represents the index of the
			 *  word for which the completion suffix will be added. This can be set to a negative value to disable the
			 *  completion suffix. */
			void completeMessage(Glib::ustring &, size_t cursor, ssize_t word_offset = 0);

			InputLine getInputLine(const Glib::ustring &) const;

			std::shared_ptr<PingPong::IRC> irc() const;

			void tabComplete();

		private:
			RedenWindow &window;
	};
}
