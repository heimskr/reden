#include <iomanip>
#include <iostream>
#include <sstream>

#include "ui/LineView.h"

namespace Reden {
	LineView::LineView(): Gtk::TextView() {
		add_css_class("lineview");
		set_editable(false);
		set_cursor_visible(false);
		auto time_tag = get_buffer()->create_tag("timestamp");
		auto bracket_tag = get_buffer()->create_tag("name_bracket");
		auto name_tag = get_buffer()->create_tag("name");
		auto message_tag = get_buffer()->create_tag("message");
		auto plain_tag = get_buffer()->create_tag("plain");
	}

	LineView & LineView::operator+=(const std::string &text) {
		auto &buffer = *get_buffer();
		addNewline();
		addTime();
		buffer.insert_with_tag(buffer.end(), text, "plain");
		return *this;
	}

	LineView & LineView::addMessage(const std::string &name, const std::string &message) {
		auto &buffer = *get_buffer();
		addNewline();
		addTime();
		buffer.insert_with_tag(buffer.end(), "<", "name_bracket");
		buffer.insert_with_tag(buffer.end(), name, "name");
		buffer.insert_with_tag(buffer.end(), ">", "name_bracket");
		buffer.insert(buffer.end(), " ");
		buffer.insert_with_tag(buffer.end(), message, "message");
		return *this;
	}

	void LineView::addNewline() {
		auto &buffer = *get_buffer();
		if (buffer.size() != 0)
			buffer.insert(buffer.end(), "\n");
	}

	void LineView::addTime() {
		auto &buffer = *get_buffer();
		buffer.insert_with_tag(buffer.end(), makeTimestamp(), "timestamp");
		buffer.insert(buffer.end(), " ");
	}

	std::string LineView::makeTimestamp(time_t now) {
		std::stringstream ss;
		tm *times = std::localtime(&now);
		ss << "[" << std::setfill('0') << std::setw(2) << times->tm_hour << ":" << std::setw(2) << times->tm_min << ":"
		   << std::setw(2) << times->tm_sec << "]";
		return ss.str();
	}

	std::string LineView::makeTimestamp() {
		return makeTimestamp(std::time(nullptr));
	}
}
