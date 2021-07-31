#include <algorithm>
#include <array>
#include <regex>
#include <string>
#include <unordered_map>

#include "ui/IRC2Pango.h"

namespace Reden {
	static std::array<Glib::ustring, 99> colorNames {
		"white", "black", "#006fb8", "#39b54a", "red", "#de382b", "#762671", "#ffc706", "yellow", "green", "#2cb5e9",
		"cyan", "blue", "magenta", "gray", "lightgray", "#330000", "#663300", "#666600", "#333300", "#003300",
		"#006633", "#003333", "#003366", "#000033", "#330066", "#330033", "#660033", "#660000", "#993300", "#999900",
		"#336600", "#006600", "#009933", "#006666", "#003399", "#000066", "#660099", "#660066", "#990033", "#990000",
		"#cc3300", "#cccc00", "#669900", "#009900", "#00ff99", "#009999", "#0066ff", "#000099", "#9900ff", "#990099",
		"#cc0033", "#ff0000", "#ff6600", "#ffff00", "#99ff00", "#00ff00", "#33ffcc", "#00ffff", "#3399ff", "#0000ff",
		"#cc33ff", "#ff00ff", "#ff0066", "#ff3333", "#ff9933", "#ffff33", "#ccff33", "#33ff33", "#66ffcc", "#33ffff",
		"#6699ff", "#3333ff", "#cc66ff", "#ff33ff", "#ff3399", "#ff9999", "#ffcc99", "#ffff99", "#ccff99", "#99ff99",
		"#99ffcc", "#99ffff", "#99ccff", "#9999ff", "#cc99ff", "#ff99ff", "#ff66cc", "#000000", "#0a0a0a", "#202020",
		"#353535", "#4a4a4a", "#606060", "#808080", "#a0a0a0", "#c0c0c0", "#eaeaea", "#ffffff",
	};

	struct Format {
		bool bold = false, underlined = false;
		int color = -1;
		Format() {}
		operator Glib::ustring() const {
			Glib::ustring out = "<span";
			if (bold)
				out += " weight=\"bold\"";
			if (0 <= color && color < 99)
				out += " color=\"" + colorNames[color] + "\"";
			return out + ">";
		}
	};

	Glib::ustring irc2pango(Glib::ustring irc) {
		using Glib::Markup::escape_text;
		enum class State {Normal, Color};

		for (gunichar ch: {'\x02', '\x03'})
			if (std::count(irc.begin(), irc.end(), ch) % 2 == 1)
				irc.push_back(ch);

		std::string color_buffer;
		Glib::ustring out;
		Glib::ustring text_buffer;
		State state = State::Normal;
		Format format;
		bool in_span = false;

		auto flush_buffer = [&] {
			out += escape_text(text_buffer);
			if (in_span)
				out += "</span>";
			else
				in_span = true;
			text_buffer.clear();
		};

		for (gunichar ch: irc) {
			if (state == State::Color) {
				if ('0' <= ch && ch <= '9' && color_buffer.size() < 2) {
					color_buffer.push_back(static_cast<char>(ch));
					if (color_buffer.size() == 2) {
						flush_buffer();
						format.color = atoi(color_buffer.c_str());
						color_buffer.clear();
						out += format;
					}
				} else
					state = State::Normal;
			}

			if (state == State::Normal) {
				switch (ch) {
					case '\x02':
						flush_buffer();
						format.bold = !format.bold;
						out += format;
						break;
					case '\x03':
						if (format.color == -1) {
							state = State::Color;
						} else {
							flush_buffer();
							format.color = -1;
							out += format;
						}
						break;
					default:
						text_buffer.push_back(ch);
						break;
				}
			}
		}

		if (!text_buffer.empty() || in_span)
			flush_buffer();
		
		static std::regex basic_span_regex("<span>([^<>]+)</span>");
		static std::regex empty_span_regex("<span[^>]*></span>");

		std::string out_string = out;
		std::string unempty, unbasic;
		std::regex_replace(std::back_inserter(unempty), out_string.begin(), out_string.end(), empty_span_regex, "");
		std::regex_replace(std::back_inserter(unbasic), unempty.begin(), unempty.end(), basic_span_regex, "$1");
		return unbasic;
	}
}
