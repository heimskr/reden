#pragma once

#include <gtkmm.h>
#include <algorithm>
#include <filesystem>
#include <list>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "lib/formicine/ansi.h"
#include "lib/formicine/futil.h"

namespace Reden::Util {
	/** Returns a path to the user's home directory as a string, optionally ending with a slash. */
	std::string getHomeString(bool append_slash = true);

	/** Returns a path to the user's home directory. */
	std::filesystem::path getHome();

	/** Escapes a string by prepending all backslashes, newlines, carriage returns, tabs, nulls and double
	 *  quotes with backslashes. */
	Glib::ustring escape(const Glib::ustring &);

	/** Unescapes a string (see spjalla::config::escape). If check_dquotes are true, the function will throw a
	 *  std::invalid_argument exception if it finds an unescaped double quote. */
	Glib::ustring unescape(const Glib::ustring &, const bool check_dquotes = true);

	/** Determines whether a message is a highlight for a given name. */
	bool isHighlight(const Glib::ustring &message, const std::string &name, bool direct_only);

	auto constexpr lower   = &formicine::util::lower;
	auto constexpr upper   = &formicine::util::upper;
	auto constexpr nthWord = &formicine::util::nth_word;
}
