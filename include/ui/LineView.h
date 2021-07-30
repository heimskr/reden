#pragma once

#include <gtkmm.h>
#include <ctime>
#include <deque>
#include <memory>
#include <vector>

#include "pingpong/core/ModeSet.h"

namespace PingPong {
	class Channel;
	class User;
}

namespace Reden {
	class LineView: public Gtk::TextView {
		public:
			LineView();

			LineView & operator+=(const std::string &text);
			LineView & addMessage(const std::string &name, const std::string &message);
			LineView & joined(const std::string &name, const std::string &channel);
			LineView & mode(std::shared_ptr<PingPong::Channel>, std::shared_ptr<PingPong::User>,
			                const PingPong::ModeSet &);
			LineView & topicChanged(std::shared_ptr<PingPong::Channel>, std::shared_ptr<PingPong::User>,
			                        const std::string &);

			bool isAlive() const { return alive; }

		private:
			Glib::RefPtr<Gtk::TextTag> timeTag, bracketTag, nameTag, messageTag, plainTag, actionTag, channelTag,
			                           modesTag, userTag, topicTag;
			bool alive = true;

			static std::string makeTimestamp(time_t);
			static std::string makeTimestamp();

			LineView & start();
			LineView & append(const std::string &text, const std::string &tag_name);
			LineView & append(const std::string &text);
			LineView & addNewline();
			LineView & addTime();
			LineView & addStar();

			void setBold(Glib::RefPtr<Gtk::TextTag>);
	};
}
