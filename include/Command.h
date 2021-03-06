#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "core/TabCompletion.h"

namespace PingPong {
	class Server;
}

namespace Reden {
	class Client;
	class InputLine;

	struct Command {
		// If the function returns true, Client::handleLine will stop iterating through the command handlers multimap.
		using BoolHandler = std::function<bool(PingPong::Server *, const InputLine &)>;
		using     Handler = std::function<void(PingPong::Server *, const InputLine &)>;

		int minArgs, maxArgs;
		bool needsServer;
		BoolHandler handler;
		Completion completionFunction;
		std::vector<CompletionState::Suggestor> suggestors;

		Command() = delete;

		Command(int min_args, int max_args, bool needs_server, const BoolHandler &handler_,
		        const Completion &completion_fn = {}, const std::vector<CompletionState::Suggestor> &suggestors_= {}):
		minArgs(min_args), maxArgs(max_args), needsServer(needs_server), handler(handler_),
		completionFunction(completion_fn), suggestors(suggestors_) {}

		Command(int min_args, int max_args, bool needs_server, const Handler &handler_,
		        const Completion &completion_fn = {}, const std::vector<CompletionState::Suggestor> &suggestors_= {}):
		minArgs(min_args), maxArgs(max_args), needsServer(needs_server), completionFunction(completion_fn),
		suggestors(suggestors_) {
			handler = [handler_](PingPong::Server *server, const InputLine &line) {
				handler_(server, line);
				return false;
			};
		}
	};

	namespace Commands {
		/** Command name, command */
		using Pair = std::pair<std::string, Command>;
	}
}
