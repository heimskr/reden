#include <deque>
#include <ostream>

#include "pingpong/core/Channel.h"

#include "core/Client.h"
#include "core/InputLine.h"
#include "core/TabCompletion.h"
#include "core/Util.h"
#include "config/Defaults.h"
#include "ui/RedenWindow.h"

namespace Reden {
	void CommandCompleter::onKey(const Key &key) {
		// If tab is pressed, complete is called before onKey, so we don't handle filling in partial here.
		if (key != KeyCode::Tab) {
			partial.clear();
			hasPartial = false;
			for (auto &pair: parent.get().completionStates)
				pair.second.reset();
		}
	}

	void CommandCompleter::complete(Glib::ustring &raw, int &cursor) {
		std::vector<Glib::ustring> split = formicine::util::split(raw, " ", true);
		Glib::ustring first = split[0].substr(1);

		if (!hasPartial) {
			partial = first;
			hasPartial = true;
		}

		std::vector<Glib::ustring> matches = parent.get().commandMatches(partial);

		// Don't bother doing anything if there are no matches.
		if (matches.size() == 0)
			return;

		const Glib::ustring rest = raw.substr(Util::lastIndexOfWord(raw, 0));

		if (matches.size() == 1) {
			raw = "/" + matches[0] + rest;
			cursor = matches[0].length() + 1;

			if (static_cast<size_t>(cursor) == raw.length())
				raw.push_back(' ');

			++cursor;
			return;
		}


		for (auto iter = matches.begin(), end = matches.end(); iter != end; ++iter) {
			if (*iter == first) {
				// The user has pressed tab again after one of the completions has been filled in. Fill in the next one.
				auto next = iter == end - 1? matches.begin() : iter + 1;
				raw = "/" + *next + rest;
				cursor = next->length() + 1;
				return;
			}
		}

		// There are multiple matches but none has been filled in yet. Start with the first.
		raw = "/" + matches[0] + rest;
		cursor = matches[0].length() + 1;
	}

	bool completeSet(Client &client_, const InputLine &line, Glib::ustring &raw, int &cursor, long arg_index,
	                 long arg_subindex) {
		CompletionState &state = client_.completionStates["set"];
		if (2 <= arg_index)
			return true;

		const Glib::ustring first_arg = line.args.empty()? "" : line.args[0];

		if (state.partial_index != arg_index) {
			state.partial = first_arg;
			state.partial_index = arg_index;
		}

		if (arg_index == 1) {
			const Glib::ustring rest = raw.substr(Util::lastIndexOfWord(raw, 1));
			const Glib::ustring piece = first_arg.substr(0, arg_subindex);
			std::vector<Glib::ustring> keys = startsWith(state.partial);
			if (keys.empty())
				return true;
			std::sort(keys.begin(), keys.end());
			Glib::ustring next = Util::nextInSequence(keys.begin(), keys.end(), piece);
			raw = "/set " + next + rest;
			cursor = next.length() + 5;
		}

		return false;
	}

	bool completePlain(Client &client_, const InputLine &, Glib::ustring &raw, int &cursor, long, long) {
		client_.completeMessage(raw, cursor, -1);
		return true;
	}

	void CompletionState::reset() {
		if (partial.empty())
			return;

		partial.clear();
		partial_index = -1;
		windex = -1;
		sindex = -1;
		cursor = -1;
	}

	bool CompletionState::empty() const {
		return partial_index == -1 && partial.empty() && windex == -1 && sindex == -1;
	}

	CompletionState::operator Glib::ustring() const {
		return "[" + std::to_string(partial_index) + "] \"" + partial + "\"";
	}

	std::ostream & operator<<(std::ostream &os, const CompletionState &state) {
		return os << Glib::ustring(state);
	}
}

namespace Reden {
	void Client::tabComplete() {
		Glib::ustring text = window.box.getInput();

		if (text.empty())
			return;

		int cursor = window.box.getCursor();

		// if (ui.activeWindow == ui.statusWindow && text[0] != '/') {
		// 	text.insert(0, "/");
		// 	window.box.setInput(text);
		// 	window.box.setCursor(++cursor);
		// 	ui.input->jumpCursor();
		// }

		InputLine il = getInputLine(text);
		auto [windex, sindex] = Util::wordIndices(text, cursor);

		if (il.isCommand()) {
			const Glib::ustring old_text(text);

			bool handled = false;

			if (windex == 0) {
				// The user wants to complete a command name.
				completer.complete(text, cursor);
			} else {
				if (windex < 0) {
					// We're not in a word, but we're where one should be.
					windex = -windex - 1;
				}

				// The user has entered a command and the cursor is at or past the first argument.

				// Search through each command handler.
				for (const auto &handler: commandHandlers) {
					const Glib::ustring &name = handler.first;
					const Command &cmd = handler.second;

					// If the command handler's name matches the command name...
					if (name == il.command) {
						// ...and the completion function is valid...
						if (cmd.completionFunction) {
							// ...then call the completion function.
							handled = cmd.completionFunction(*this, il, text, cursor, windex, sindex);
						}

						// Even if the completion function is invalid, stop the search.
						break;
					}
				}
			}

			if (!handled) {
				if (old_text != text)
					window.box.setInput(text);
				window.box.setCursor(cursor);
			}
		} else if (window.box.activeServer()) {
			if (window.box.inStatus() && text[0] != '/') {
				text.insert(0, "/");
				window.box.setInput(text);
				window.box.setCursor(++cursor);
			}

			completeMessage(text, cursor);
		}
	}

	void Client::completeMessage(Glib::ustring &text, int cursor, int word_offset) {
		if (text.empty())
			return;

		InputLine il = getInputLine(text);
		auto [windex, sindex] = Util::wordIndices(text, cursor);

		const Glib::ustring old_text(text);

		const PingPong::Server *server = window.box.activeServer();
		const PingPong::Channel *channel = window.box.activeChannel();
		const PingPong::User *user = window.box.activeUser();

		CompletionState &state = completionStates["_"];
		Glib::ustring word;
		if (!state.empty()) {
			windex = state.windex;
			sindex = state.sindex;
			word = Util::nthWord(text, windex);
		} else {
			word = Util::nthWord(text, windex);
			if (!word.empty()) {
				state.windex = windex;
				state.sindex = sindex;
				state.partial = word;
			} else {
				return;
			}
		}

		const Glib::ustring &suffix = config.getString("completion", "ping_suffix");
		formicine::util::remove_suffix(word, suffix);

		std::deque<Glib::ustring> items;
		bool do_sort = true; // For channels, the order is important and shouldn't be disturbed.

		const Glib::ustring lower = state.partial.lowercase();

		if (word[0] == '#') {
			for (const std::shared_ptr<PingPong::Channel> &ptr: server->channels) {
				if (Glib::ustring(ptr->name).lowercase().find(lower) == 0)
					items.push_back(ptr->name);
			}
		} else if (channel) {
			do_sort = false;
			for (const std::shared_ptr<PingPong::User> &ptr: channel->users) {
				if (Glib::ustring(ptr->name).lowercase().find(lower) == 0)
					items.push_back(ptr->name);
			}
		} else if (user) {
			items.push_back(user->name);
			items.push_back(server->getNick());
		} else {
			return;
		}

		if (!items.empty()) {
			if (do_sort)
				formicine::util::insensitive_sort(items.begin(), items.end());

			if (1 < items.size()) {
				// It's reasonable to assume that you're not desparate to complete your own nick, so for your
				// convenience we'll move your name to the end of the list.
				const Glib::ustring self = server->getNick();
				for (auto iter = items.begin(); iter != items.end() && iter + 1 != items.end(); ++iter)
					if (*iter == self) {
						items.erase(iter);
						items.push_back(self);
						break;
					}
			}

			const Glib::ustring next = Util::nextInSequence(items.begin(), items.end(), word);
			cursor = Util::replaceWord(text, windex, next);

			if (windex == word_offset && next[0] != '#' && !suffix.empty()) {
				// Erase any space already after the colon to prevent spaces from accumulating.
				if (static_cast<size_t>(cursor) < text.length() && std::isspace(text[cursor]))
					text.erase(cursor, 1);

				text.insert(cursor, suffix + " ");
				cursor += suffix.length() + 1;
			}
		}

		if (old_text != text)
			window.box.setInput(text);

		window.box.setCursor(cursor);
	}
}
