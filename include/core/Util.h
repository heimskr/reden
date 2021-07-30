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

	/** Escapes a string by prepending all backslashes, newlines, carriage returns, tabs, nulls and double quotes with
	 *  backslashes. */
	Glib::ustring escape(const Glib::ustring &);

	/** Unescapes a string (see spjalla::config::escape). If check_dquotes is true, the function will throw a
	 *  std::invalid_argument exception if it finds an unescaped double quote. */
	Glib::ustring unescape(const Glib::ustring &, const bool check_dquotes = true);

	/** Determines whether a message is a highlight for a given name. */
	bool isHighlight(const Glib::ustring &message, const std::string &name, bool direct_only);

	/** Returns the index of the first character in the n-th word of a string. If n is greater than the number of words
	 *  in the string, the length of the string is returned. */
	size_t indexOfWord(const Glib::ustring &, size_t n);

	/** Replaces the n-th word of a string with another string and returns the position of the first character after the
	 *  replacement word (or -1 if the string has no n-th word). */
	ssize_t replaceWord(Glib::ustring &, size_t n, const Glib::ustring &);

	/** Returns the index of the word that a given index is in in addition to the index within the word.
	 *  If the cursor is within a group of multiple spaces between two words, the first value will be negative.
	 *  If the first value is -1, the cursor is before the first word. -2 indicates that the cursor is before the
	 *  second word, -3 before the third, and so on. The second value will be -1 if the first value is negative. */
	std::pair<ssize_t, ssize_t> wordIndices(const Glib::ustring &, size_t);

	Glib::ustring nthWord(const Glib::ustring &, size_t, bool condense = false);

	/** Returns the index of first character after the n-th word of a string. If n is greater than the number of
	 *  words in the string, the length of the string is returned. */
	size_t lastIndexOfWord(const Glib::ustring &, size_t n);

	/** Treats a range like a circular buffer and finds the next value after a given value. If the given value isn't
	 *  present in the range, the function returns the value at the beginning of the range. If the range is empty, the
	 *  function throws std::invalid_argument. */
	template <typename Iter>
	Glib::ustring & nextInSequence(Iter begin, Iter end, const Glib::ustring &str) {
		if (begin == end)
			throw std::invalid_argument("Empty string");

		for (Iter iter = begin; iter != end; ++iter) {
			if (*iter == str) {
				++iter;
				return iter == end? *begin : *iter;
			}
		}

		return *begin;
	}

	template <typename T>
	size_t nthIndex(const Glib::ustring &str, const T &to_find, int n) {
		size_t index = 0;
		for (int i = 0; i < n; ++i)
			index = str.find(to_find, i? index + 1 : i);
		return index;
	}
}
