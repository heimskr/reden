#include <iomanip>
#include <iostream>
#include <sstream>

#include "ui/LineView.h"
#include "pingpong/core/Channel.h"

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
		  modesTag = buffer.create_tag("modes");
		   userTag = buffer.create_tag("user"); // For things like mode lines
		  topicTag = buffer.create_tag("topic");
		bracketTag->property_foreground() = "gray";
		   timeTag->property_foreground() = "gray";
		   timeTag->property_font() = "Monospace";
		setBold(nameTag);
		setBold(actionTag);
		setBold(channelTag);
		setBold(userTag);
		setBold(topicTag);
	}

	LineView & LineView::operator+=(const std::string &text) {
		return start().append(text, "plain");
	}

	LineView & LineView::addMessage(const std::string &name, const std::string &message) {
		start().append("<", "name_bracket").append(name, "name").append(">", "name_bracket").append(" ");
		return append(message, "message");
	}

	LineView & LineView::joined(const std::string &name, const std::string &channel) {
		return start().addStar().append(name, "name").append(" joined ").append(channel, "channel");
	}

	LineView & LineView::mode(std::shared_ptr<PingPong::Channel> channel, std::shared_ptr<PingPong::User> who,
	                          const PingPong::ModeSet &modeset) {
		start().addStar();
		if (!who) {
			append("Mode" + std::string(modeset.count() == 1? "" : "s") + " set: ");
		} else {
			append(channel->withHat(who), "user");
			append(" set mode" + std::string(modeset.count() == 1? "" : "s") + " ");
		}
		append(modeset.modeString(), "modes");
		if (!modeset.extra.empty())
			append(" on ").append(modeset.extra, "user");
		return *this;
	}

	LineView & LineView::topicChanged(std::shared_ptr<PingPong::Channel> channel, std::shared_ptr<PingPong::User> who,
	                                  const std::string &topic) {
		start();
		if (!who)
			append("Topic for ").append(channel->name, "channel").append(" is ").append(topic, "topic");
		else
			append(channel->withHat(who), "user").append(" changed the topic to ").append(topic, "topic");
		return *this;
	}

	void LineView::clear() {
		get_buffer()->set_text("");
	}

	LineView & LineView::start() {
		return addNewline().addTime();
	}

	LineView & LineView::append(const std::string &text, const std::string &tag_name) {
		get_buffer()->insert_with_tag(get_buffer()->end(), text, tag_name);
		return *this;
	}

	LineView & LineView::append(const std::string &text) {
		get_buffer()->insert(get_buffer()->end(), text);
		return *this;
	}

	LineView & LineView::addNewline() {
		auto &buffer = *get_buffer();
		if (buffer.size() != 0)
			buffer.insert(buffer.end(), "\n");
		return *this;
	}

	LineView & LineView::addTime() {
		auto &buffer = *get_buffer();
		buffer.insert_with_tag(buffer.end(), makeTimestamp(), "timestamp");
		buffer.insert(buffer.end(), " ");
		return *this;
	}

	LineView & LineView::addStar() {
		append("*", "action").append(" ");
		return *this;
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

	void LineView::setBold(Glib::RefPtr<Gtk::TextTag> tag) {
		tag->property_weight() = 2 * tag->property_weight();
	}
}
