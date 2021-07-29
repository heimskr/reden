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

		private:
			static std::string makeTimestamp(time_t);
			static std::string makeTimestamp();

			void addNewline();
			void addTime();
	};
}
