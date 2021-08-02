#include <vector>

#include "tests/Tests.h"
#include "ui/IRC2Pango.h"

#include "lib/formicine/ansi.h"

namespace Reden::Tests {
	bool irc2pangoTests() {
		static std::vector<std::pair<Glib::ustring, Glib::ustring>> tests {
			{"\x03" "4,11Foo \x02" "Bar\x02 Baz \x03" "8,9 Quux\x03", "<span foreground=\"red\" background=\"cyan\">Foo </span><span weight=\"bold\" foreground=\"red\" background=\"cyan\">Bar</span><span foreground=\"red\" background=\"cyan\"> Baz </span><span foreground=\"yellow\" background=\"green\"> Quux</span>"},
			{"Foo", "Foo"},
			{"Foo\x02" "Bar", "Foo<span weight=\"bold\">Bar</span>"},
			{"\x03" "04foo\x02" "bar\x03" "baz\x02", "<span foreground=\"red\">foo</span><span weight=\"bold\" foreground=\"red\">bar</span><span weight=\"bold\">baz</span>"},
			{"\x02" "Foo", "<span weight=\"bold\">Foo</span>"},
			{"\x02" "Foo\x02", "<span weight=\"bold\">Foo</span>"},
			{"\x02" "Foo\x03", "<span weight=\"bold\">Foo</span>"},
			{"\x02\x03\x02\x03\x03\x02", ""},
			{"\x03" "04Foo \x02" "Bar\x02 Baz \x03" "08 Quux\x03", "<span foreground=\"red\">Foo </span><span weight=\"bold\" foreground=\"red\">Bar</span><span foreground=\"red\"> Baz </span><span foreground=\"yellow\"> Quux</span>"},
			{"\x03" "4Foo \x02" "Bar\x02 Baz \x03" "8 Quux\x03", "<span foreground=\"red\">Foo </span><span weight=\"bold\" foreground=\"red\">Bar</span><span foreground=\"red\"> Baz </span><span foreground=\"yellow\"> Quux</span>"},
		};

		if (tests.empty()) {
			ansi::out << ansi::warn << "No irc2pango tests." << ansi::endl;
			return false;
		}

		ansi::out << ansi::info << "Testing irc2pango." << ansi::endl;
		size_t passed = 0, failed = 0;
		for (const auto &[input, expected]: tests) {
			Glib::ustring input_escaped;
			for (gunichar ch: input)
				switch (ch) {
					case '\x02': input_escaped += "\\x02"; break;
					case '\x03': input_escaped += "\\x03"; break;
					default: input_escaped.push_back(ch);
				}
			Glib::ustring actual = irc2pango(input);
			if (actual == expected) {
				ansi::out << ansi::good << ansi::bold(input_escaped) << " => "_d << ansi::bold(expected) << ansi::endl;
				++passed;
			} else {
				ansi::out << ansi::bad << ansi::bold(input_escaped) << ansi::endl;
				ansi::out << "    Expected: " << ansi::bold(expected) << ansi::endl;
				ansi::out << "    Actual:   " << ansi::bold(actual) << ansi::endl;
				++failed;
			}
		}

		if (passed && !failed) {
			if (passed == 1)
				ansi::out << ansi::good << "Passed one test." << ansi::endl;
			else
				ansi::out << ansi::good << "Passed all " << passed << " tests." << ansi::endl;
			return true;
		} else if (failed && !passed) {
			if (failed == 1)
				ansi::out << ansi::bad << "Failed one test." << ansi::endl;
			else
				ansi::out << ansi::bad << "Failed all " << failed << " tests." << ansi::endl;
			return false;
		} else {
			ansi::out << ansi::warn << "Passed " << passed << " test" << (passed == 1? "" : "s") << " and failed " 
			          << failed << " test" << (failed == 1? "" : "s") << "." << ansi::endl;
			return false;
		}
	}
}