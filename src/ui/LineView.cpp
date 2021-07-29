#include <iomanip>
#include <iostream>
#include <sstream>

#include "ui/LineView.h"

namespace Reden {
	LineView::LineView(): Gtk::TextView() {
		add_css_class("lineview");
		set_editable(false);
		set_cursor_visible(false);
		set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
		auto &buffer = *get_buffer();
		   timeTag = buffer.create_tag("timestamp");
		bracketTag = buffer.create_tag("name_bracket");
		   nameTag = buffer.create_tag("name");
		messageTag = buffer.create_tag("message");
		  plainTag = buffer.create_tag("plain");
		 actionTag = buffer.create_tag("action");
		channelTag = buffer.create_tag("channel");
		bracketTag->property_foreground() = "gray";
		   timeTag->property_foreground() = "gray";
		   nameTag->property_weight()     = nameTag->property_weight() * 2;
		 actionTag->property_weight()     = actionTag->property_weight() * 2;
		channelTag->property_weight()     = channelTag->property_weight() * 2;
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
