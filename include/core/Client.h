#pragma once

#include <gtkmm.h>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "Command.h"
#include "config/Cache.h"
#include "config/ConfigDB.h"
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
			Client(RedenWindow &window_): window(window_), completer(*this), configs(*this, false) {}

			Client & operator=(const Client &) = delete;

			/** Tries to expand a command (e.g., "mod" → "mode"). Returns a vector of all matches. */
			std::vector<Glib::ustring> commandMatches(const Glib::ustring &);

			/** Completes a message for a given cursor position. The word_offset parameter represents the index of the
			 *  word for which the completion suffix will be added. This can be set to a negative value to disable the
			 *  completion suffix. */
			void completeMessage(Glib::ustring &, int cursor, int word_offset = 0);

			InputLine getInputLine(const Glib::ustring &) const;

			/** Processes a line of user input and returns whether the line was recognized as a valid input. */
			bool handleLine(const InputLine &);

			std::shared_ptr<PingPong::IRC> irc() const;

			void tabComplete();

		private:
			RedenWindow &window;
			CommandCompleter completer;
			ConfigDB configs;
			std::multimap<Glib::ustring, Command> commandHandlers;
	};
}
