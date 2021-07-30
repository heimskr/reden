#pragma once

#include <gtkmm.h>
#include <string>
#include <vector>

#include "lib/formicine/futil.h"

namespace Reden {
	class InputLine {
		private:
			bool minimal = false; // Whether the command is "/".

		public:
			Glib::ustring command, body;
			std::vector<Glib::ustring> args;
			Glib::ustring original;

			InputLine() = delete;
			InputLine(const Glib::ustring &command_, const Glib::ustring &body_):
				command(formicine::util::lower(command_)), body(body_), original("/" + command_ + " " + body_) {}
			InputLine(const Glib::ustring &full);

			inline bool isCommand() const { return minimal || !command.empty(); }
			Glib::ustring first() const;
			Glib::ustring rest() const;
			operator Glib::ustring() const;

			friend std::ostream & operator<<(std::ostream &, const InputLine &);
	};
}
