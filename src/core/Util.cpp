#include <cstdlib>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "core/Util.h"
#include "pingpong/core/Util.h"

namespace Reden::Util {
	std::string getHomeString(bool append_slash) {
		const char *home = getenv("HOME");
		if (home == nullptr)
			home = getpwuid(getuid())->pw_dir;
		std::string str = home;
		if (append_slash && !str.empty() && str.back() != '/' && str.back() != '\\')
			str.push_back('/');
		return str;
	}

	std::filesystem::path getHome() {
		return {getHomeString(false)};
	}

	Glib::ustring escape(const Glib::ustring &str) {
		std::ostringstream out;
		for (char ch: str) {
			switch (ch) {
				case '\\': out << "\\\\"; break;
				case '\n': out << "\\n"; break;
				case '\r': out << "\\r"; break;
				case '\t': out << "\\t"; break;
				case '\0': out << "\\0"; break;
				case '"':  out << "\""; break;
				default:   out << ch;
			}
		}

		return out.str();
	}

	Glib::ustring unescape(const Glib::ustring &str, const bool check_dquotes) {
		std::ostringstream out;
		for (size_t i = 0, length = str.length(); i < length; ++i) {
			char ch = str[i];

			// Looking at the next character when we're at the end of the string would be bad.
			if (i == length - 1) {
				out << ch;
				break;
			}

			if (ch == '\\') {
				switch (str[i + 1]) {
					case '\\': out << "\\"; ++i; break;
					case 'n':  out << "\n"; ++i; break;
					case 'r':  out << "\r"; ++i; break;
					case 't':  out << "\t"; ++i; break;
					case '0':  out << "\0"; ++i; break;
					case '"':  out << "\""; ++i; break;
				}
			} else if (check_dquotes && ch == '"') {
				throw std::invalid_argument("String contains an unescaped double quote");
			} else {
				out << ch;
			}
		}

		return out.str();
	}

	bool isHighlight(const Glib::ustring &message, const std::string &name, bool direct_only) {
		const std::string lmessage = formicine::util::lower(message);
		const std::string lname = formicine::util::lower(name);

		if (lmessage.find(lname + ":") == 0 || lmessage.find(lname + ",") == 0 || lmessage.find(lname + " ") == 0)
			return true;

		if (!direct_only) {
			std::string filtered = formicine::util::filter(lmessage, std::string(" ") + PingPong::Util::nickChars);
			std::vector<std::string> words = formicine::util::split(filtered, " ", true);
			for (const std::string &word: words) {
				if (word == lname)
					return true;
			}
		}

		return false;
	}
}
