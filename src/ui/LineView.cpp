#include <iomanip>
#include <iostream>
#include <cmath>
#include <sstream>

#include "core/Util.h"
#include "ui/IRC2Pango.h"
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
		 bracketTag = buffer.create_tag("bracket");
		    nameTag = buffer.create_tag("name");
		 messageTag = buffer.create_tag("message");
		   plainTag = buffer.create_tag("plain");
		  actionTag = buffer.create_tag("action");
		 channelTag = buffer.create_tag("channel");
		   modesTag = buffer.create_tag("modes");
		    userTag = buffer.create_tag("user"); // For things like mode lines
		   topicTag = buffer.create_tag("topic");
		asteriskTag = buffer.create_tag("asterisk");
		    selfTag = buffer.create_tag("self");
		bracketTag->property_foreground() = "gray";
		   timeTag->property_foreground() = "gray";
		   timeTag->property_font() = "Monospace";
		setBold(nameTag);
		setBold(actionTag);
		setBold(channelTag);
		setBold(userTag);
		setBold(topicTag);
		setBold(selfTag);
		endMark = get_buffer()->create_mark(get_buffer()->end(), false);
	}

	LineView & LineView::add(const Glib::ustring &text, bool pangoize) {
		start();
		if (pangoize)
			appendMarkup(irc2pango(text));
		else
			append(text, "plain");
		return scroll();
	}

	LineView & LineView::addMessage(const Glib::ustring &name, const Glib::ustring &message, bool is_self) {
		return start().addMessageMain(name, message, is_self);
	}

	LineView & LineView::addMessage(const Glib::ustring &name, const Glib::ustring &message, bool is_self, int hour,
	                                int minute, int second) {
		wasAtEnd = atEnd();
		return addNewline().addTime(makeTimestamp(hour, minute, second)).addMessageMain(name, message, is_self);
	}

	LineView & LineView::joined(const Glib::ustring &name, const Glib::ustring &channel) {
		return start().addStar().append(name, "name").append(" joined ").append(channel, "channel").scroll();
	}

	LineView & LineView::parted(const Glib::ustring &name, const Glib::ustring &channel, const Glib::ustring &reason) {
		start().addStar().append(name, "name").append(" left ").append(channel, "channel");
		if (!reason.empty())
			append(" ").append("[", "bracket").appendMarkup(irc2pango(reason)).append("]", "bracket");
		return scroll();
	}

	LineView & LineView::quit(const Glib::ustring &name, const Glib::ustring &reason) {
		start().addStar().append(name, "name").append(" quit");
		if (!reason.empty())
			append(" ").append("[", "bracket").appendMarkup(irc2pango(reason)).append("]", "bracket");
		return scroll();
	}

	LineView & LineView::mode(std::shared_ptr<PingPong::Channel> channel, std::shared_ptr<PingPong::User> who,
	                          const PingPong::ModeSet &modeset) {
		start().addStar();
		if (!who) {
			append("Mode" + Glib::ustring(modeset.count() == 1? "" : "s") + " set: ");
		} else {
			append(channel->withHat(who), "user");
			append(" set mode" + Glib::ustring(modeset.count() == 1? "" : "s") + " ");
		}
		append(modeset.modeString(), "modes");
		if (!modeset.extra.empty())
			append(" on ").append(modeset.extra, "user");
		return scroll();
	}

	LineView & LineView::topicChanged(std::shared_ptr<PingPong::Channel> channel, std::shared_ptr<PingPong::User> who,
	                                  const Glib::ustring &topic) {
		start();
		if (!who)
			append("Topic for ").append(channel->name, "channel").append(" is ").append(topic, "topic");
		else
			append(channel->withHat(who), "user").append(" changed the topic to ").append(topic, "topic");
		return scroll();
	}

	void LineView::clear() {
		get_buffer()->set_text("");
	}

	PingPong::Channel * LineView::getChannel() const {
		return type == ParentType::Channel? static_cast<PingPong::Channel *>(parent) : nullptr;
	}

	PingPong::Server * LineView::getServer() const {
		return type == ParentType::Server? static_cast<PingPong::Server *>(parent) : nullptr;
	}

	PingPong::User * LineView::getUser() const {
		return type == ParentType::User? static_cast<PingPong::User *>(parent) : nullptr;
	}

	LineView & LineView::set(PingPong::Channel *channel) {
		parent = channel;
		type = ParentType::Channel;
		return *this;
	}

	LineView & LineView::set(PingPong::Server *server) {
		parent = server;
		type = ParentType::Server;
		return *this;
	}

	LineView & LineView::set(PingPong::User *user) {
		parent = user;
		type = ParentType::User;
		return *this;
	}

	Glib::ustring LineView::makeTimestamp(int hour, int minute, int second) {
		std::stringstream ss;
		ss << "[" << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2) << minute << ":" << std::setw(2)
		   << second << "]";
		return ss.str();
	}

	Glib::ustring LineView::makeTimestamp(time_t now) {
		tm *times = std::localtime(&now);
		return makeTimestamp(times->tm_hour, times->tm_min, times->tm_sec);
	}

	Glib::ustring LineView::makeTimestamp() {
		return makeTimestamp(std::time(nullptr));
	}

	LineView & LineView::start() {
		wasAtEnd = atEnd();
		return addNewline().addTime();
	}

	LineView & LineView::append(const Glib::ustring &text, const Glib::ustring &tag_name) {
		get_buffer()->insert_with_tag(get_buffer()->end(), text, tag_name);
		return *this;
	}

	LineView & LineView::append(const Glib::ustring &text) {
		get_buffer()->insert(get_buffer()->end(), text);
		return *this;
	}

	LineView & LineView::appendMarkup(const Glib::ustring &markup) {
		get_buffer()->insert_markup(get_buffer()->end(), markup);
		return *this;
	}

	LineView & LineView::addNewline() {
		auto &buffer = *get_buffer();
		if (buffer.size() != 0)
			buffer.insert(buffer.end(), "\n");
		return *this;
	}

	LineView & LineView::addTime() {
		return addTime(makeTimestamp());
	}

	LineView & LineView::addTime(const Glib::ustring &timestamp) {
		auto &buffer = *get_buffer();
		buffer.insert_with_tag(buffer.end(), timestamp, "timestamp");
		buffer.insert(buffer.end(), " ");
		return *this;
	}

	LineView & LineView::addStar() {
		append("*", "asterisk").append(" ");
		return *this;
	}

	LineView & LineView::addMessageMain(const Glib::ustring &name, const Glib::ustring &message, bool is_self) {
		if (Util::isAction(message)) {
			Glib::ustring copy = message;
			Util::trimAction(copy);
			addStar().append(name[0] == ' '? name.substr(1) : name, "name").append(" ").appendMarkup(irc2pango(copy));
			return scroll();
		}

		append("<", "bracket");
		if (is_self)
			append(name, "self");
		else
			append(name);
		return append(">", "bracket").append(" ").appendMarkup(irc2pango(message)).scroll();


		if (Util::isAction(message)) {
			Glib::ustring copy = message;
			Util::trimAction(copy);
			addStar().append(name[0] == ' '? name.substr(1) : name, "name").append(" ").appendMarkup(irc2pango(copy));
			return scroll();
		}

		append("<", "bracket");
		if (is_self)
			append(name, "self");
		else
			append(name);
		return append(">", "bracket").append(" ").appendMarkup(irc2pango(message)).scroll();
	}

	void LineView::setBold(Glib::RefPtr<Gtk::TextTag> tag) {
		tag->property_weight() = 2 * tag->property_weight();
	}

	bool LineView::atEnd() const {
		const auto vadj = get_vadjustment();
		return std::abs(vadj->get_upper() - vadj->get_page_size() - vadj->get_value()) < 0.0001;
	}

	LineView & LineView::scroll() {
		if (wasAtEnd)
			scroll_to(endMark);
		return *this;
	}
}
