#include <iomanip>
#include <iostream>
#include <sstream>

#include "ui/LineView.h"

namespace Reden {
	LineView::LineView(): Gtk::TextView() {
		add_css_class("lineview");
		set_editable(false);
		set_cursor_visible(false);
		auto &buffer = *get_buffer();
		auto    time_tag = buffer.create_tag("timestamp");
		auto bracket_tag = buffer.create_tag("name_bracket");
		auto    name_tag = buffer.create_tag("name");
		auto message_tag = buffer.create_tag("message");
		auto   plain_tag = buffer.create_tag("plain");
		auto  action_tag = buffer.create_tag("action");
		auto channel_tag = buffer.create_tag("channel");
		bracket_tag->property_foreground() = "gray";
		   time_tag->property_foreground() = "gray";
		   name_tag->property_weight()     = name_tag->property_weight() * 2;
		 action_tag->property_weight()     = action_tag->property_weight() * 2;
		channel_tag->property_weight()     = channel_tag->property_weight() * 2;
	}

	LineView & LineView::operator+=(const std::string &text) {
		start();
		return append(text, "plain");
	}

	LineView & LineView::addMessage(const std::string &name, const std::string &message) {
		start();
		append("<", "name_bracket").append(name, "name").append(">", "name_bracket").append(" ");
		return append(message, "message");
	}

	LineView & LineView::joined(const std::string &name, const std::string &channel) {
		start();
		return append("*", "action").append(" ").append(name, "name").append(" joined ").append(channel, "channel");
	}

	Gtk::TextBuffer & LineView::start() {
		auto &buffer = *get_buffer();
		addNewline();
		addTime();
		return buffer;
	}

	LineView & LineView::append(const std::string &text, const std::string &tag_name) {
		get_buffer()->insert_with_tag(get_buffer()->end(), text, tag_name);
		return *this;
	}

	LineView & LineView::append(const std::string &text) {
		get_buffer()->insert(get_buffer()->end(), text);
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
