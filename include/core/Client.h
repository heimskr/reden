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

#include "pingpong/core/Server.h"

namespace PingPong {
	class IRC;
}

namespace Reden {
	class RedenWindow;

	class Client {
		public:
			std::unordered_map<std::string, CompletionState> completionStates;
			ConfigDB config;
			ConfigCache cache;

			Client() = delete;
			Client(const Client &) = delete;
			Client(RedenWindow &window_);

			Client & operator=(const Client &) = delete;

			/** Adds a command handler, given a pair that signifies the name of the command as typed by the user plus a
			 *  handler tuple. */
			void add(const Commands::Pair &);
			void add(const std::string &, const Command &);
			void add(const std::string &, int, int, bool, const Command::Handler &, const Completion & = {},
			         const std::vector<CompletionState::Suggestor> & = {});
			void addBool(const std::string &, int, int, bool, const Command::BoolHandler &, const Completion & = {},
			             const std::vector<CompletionState::Suggestor> & = {});

			/** Tries to expand a command (e.g., "mod" → "mode"). Returns a vector of all matches. */
			std::vector<Glib::ustring> commandMatches(const Glib::ustring &);

			/** Completes a message for a given cursor position. The word_offset parameter represents the index of the
			 *  word for which the completion suffix will be added. This can be set to a negative value to disable the
			 *  completion suffix. */
			void completeMessage(Glib::ustring &, int cursor, int word_offset = 0);

			InputLine getInputLine(const Glib::ustring &) const;

			/** Processes a line of user input and returns whether the line was recognized as a valid input. */
			bool handleLine(const InputLine &);

			void init();

			std::shared_ptr<PingPong::IRC> irc() const;

			void tabComplete();

		private:
			RedenWindow &window;
			CommandCompleter completer;
			std::multimap<Glib::ustring, Command> commandHandlers;

// client/Commands.cpp

		public:
			/** Adds the built-in command handlers. */
			void addCommands();

// client/Events.cpp

		private:
			using QueueFn   = std::function<void()>;
			using QueuePair = std::pair<PingPong::Server::Stage, QueueFn>;
			std::unordered_map<PingPong::Server *, std::list<QueuePair>> serverStatusQueue {};

			/** Calls and removes all functions in the server status queue waiting for a given server and status. */
			void callInQueue(PingPong::Server *, PingPong::Server::Stage);

		public:
			/** Adds listeners for pingpong events. */
			void addEvents();

			/** Adds a function to a queue to be called when a server reaches a given stage. */
			void waitForServer(PingPong::Server *, PingPong::Server::Stage, const QueueFn &);
	};
}
