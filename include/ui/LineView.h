#pragma once

#include <gtkmm.h>
#include <deque>
#include <memory>
#include <vector>

namespace Reden {
	class LineView: public Gtk::Grid {
		public:
			LineView();

			LineView & operator+=(const std::string &);

		private:
			std::deque<std::vector<std::unique_ptr<Gtk::Widget>>> widgets;
			int row = 0;
	};
}
