#pragma once

#include <deque>
#include <functional>
#include <string>
#include <vector>

#include "core/Key.h"
#include "core/InputLine.h"

namespace Reden {
	class Client;

	/** Carries out the tab completion logic for a given input line. The raw string is passed as a reference that can be
	 *  modified to add completion data. The cursor index is also passed too, and it should also be updated.
	 *  For convenience, the index of the argument the cursor is in is provided, as well as the index within the
	 *  argument. If either of them doesn't apply, they'll be negative. The function should return true if it handled
	 *  updating the textinput's text and jumping to the cursor itself, or false if Client::tabComplete should do it.
	 */
	using Completion = std::function<bool(Client &, const InputLine &, Glib::ustring &raw, size_t &cursor,
	                                      long arg_index, long sub)>;

	/** Completes the /set command. */
	bool completeSet(Client &, const InputLine &, Glib::ustring &raw, size_t &cursor, long arg_index, long sub);

	/** Completes nicknames but doesn't add ping suffixes. */
	bool completePlain(Client &, const InputLine &, Glib::ustring &raw, size_t &cursor, long arg_index, long sub);

	/** Contains the state data and logic for dealing with some parts of tab completion for commands. Clients keep an
	 *  instance of this and pass keypresses to it. */
	class CommandCompleter {
		private:
			std::reference_wrapper<Client> parent;

			/** When the user types a partial command and presses tab, it's stored here. If they press any key other
			 *  than tab, it's cleared. */
			Glib::ustring partial;

			bool hasPartial = false;

		public:
			CommandCompleter(Client &parent_): parent(parent_) {}

			void onKey(const Key &);
			void complete(Glib::ustring &, size_t &);
	};

	struct CompletionState {
		using Suggestor = std::function<Glib::ustring(const std::vector<Glib::ustring> &)>;

		Glib::ustring partial;
		ssize_t partial_index = -1;
		ssize_t windex = -1;
		ssize_t sindex = -1;
		ssize_t cursor = -1;

		std::vector<Suggestor> suggestors {};
		
		template <typename T>
		CompletionState(const T &suggestors_): suggestors(suggestors_.begin(), suggestors_.end()) {}

		CompletionState(const std::vector<Suggestor> &suggestors_ = {}): suggestors(suggestors_) {}

		/** Resets the partial string and partial index. */
		void reset();

		/** Returns whether nothing has been set (or if reset() has just been called). */
		bool empty() const;

		operator Glib::ustring() const;
	};
}
