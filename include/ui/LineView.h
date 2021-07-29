#pragma once

#include <gtkmm.h>
#include <ctime>
#include <deque>
#include <memory>
#include <vector>

namespace Reden {
	class LineView: public Gtk::TextView {
		public:
			LineView();

			LineView & operator+=(const std::string &text);
			LineView & addMessage(const std::string &name, const std::string &message);
			LineView & joined(const std::string &name, const std::string &channel);

		private:
			Glib::RefPtr<Gtk::TextTag> timeTag, bracketTag, nameTag, messageTag, plainTag, actionTag, channelTag;
			static std::string makeTimestamp(time_t);
			static std::string makeTimestamp();


			Gtk::TextBuffer & start();
			LineView & append(const std::string &text, const std::string &tag_name);
			LineView & append(const std::string &text);
			void addNewline();
			void addTime();
	};
}
