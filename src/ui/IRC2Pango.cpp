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
		int foreground = -1, background = -1;
		Format() {}
		bool isFormatted() const {
			return bold || underlined || foreground != -1 || background != -1;
		}
		operator Glib::ustring() const {
			Glib::ustring out = "<span";
			if (bold)
				out += " weight=\"bold\"";
			if (underlined)
				out += " underline=\"single\"";
			if (0 <= foreground && foreground < 99)
				out += " foreground=\"" + colorNames[foreground] + "\"";
			if (0 <= background && background < 99)
				out += " background=\"" + colorNames[background] + "\"";
			return out + ">";
		}
	};

	// I'm sort of sorry for this.
	Glib::ustring irc2pango(Glib::ustring irc) {
		using Glib::Markup::escape_text;
		enum class State {Normal, ExitForeground, Foreground, Background};

		for (gunichar ch: {'\x02', '\x03', '\x1f'})
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
			if (in_span) {
				out += "</span>";
				in_span = false;
			}
			text_buffer.clear();
		};

		auto add_format = [&] {
			if (format.isFormatted()) {
				out += format;
				in_span = true;
			}
		};

		for (gunichar ch: irc) {
			if (state == State::ExitForeground) {
				if (ch == ',') {
					state = State::Background;
					continue;
				} else {
					state = State::Normal;
					flush_buffer();
					add_format();
				}
			}

			if (state == State::Foreground) {
				if ('0' <= ch && ch <= '9' && color_buffer.size() < 2) {
					color_buffer.push_back(static_cast<char>(ch));
					if (color_buffer.size() == 2) {
						format.foreground = atoi(color_buffer.c_str());
						color_buffer.clear();
						state = State::ExitForeground;
						continue;
					}
				} else {
					if (!color_buffer.empty()) {
						flush_buffer();
						format.foreground = atoi(color_buffer.c_str());
					}
					color_buffer.clear();
					if (ch == ',') {
						state = State::Background;
					} else {
						state = State::Normal;
						if (format.foreground != -1)
							add_format();
					}
				}
			} else if (state == State::Background) {
				if ('0' <= ch && ch <= '9' && color_buffer.size() < 2) {
					color_buffer.push_back(static_cast<char>(ch));
					if (color_buffer.size() == 2) {
						flush_buffer();
						format.background = atoi(color_buffer.c_str());
						color_buffer.clear();
						add_format();
						state = State::Normal;
						continue;
					}
				} else {
					if (!color_buffer.empty()) {
						flush_buffer();
						format.background = atoi(color_buffer.c_str());
					}
					color_buffer.clear();
					state = State::Normal;
					if (format.background != -1)
						add_format();
				}
			}

			if (state == State::Normal) {
				switch (ch) {
					case '\x02':
						flush_buffer();
						format.bold = !format.bold;
						add_format();
						break;
					case '\x03':
						if (format.foreground == -1 && format.background == -1) {
							state = State::Foreground;
						} else {
							flush_buffer();
							state = State::Foreground;
							format.foreground = -1;
							format.background = -1;
							add_format();
						}
						break;
					case '\x1f':
						flush_buffer();
						format.underlined = !format.underlined;
						add_format();
						break;
					default:
						text_buffer.push_back(ch);
						break;
				}
			}
		}

		if (!text_buffer.empty() || in_span)
			flush_buffer();
		
		static std::regex empty_span_regex("<span[^>]*></span>");
		const std::string out_string = out;
		std::string unempty;
		std::regex_replace(std::back_inserter(unempty), out_string.begin(), out_string.end(), empty_span_regex, "");
		return unempty;
	}
}
