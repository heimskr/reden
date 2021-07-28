#include <iomanip>
#include <sstream>
#include <ctime>

#include "ui/LineView.h"

namespace Reden {
	LineView::LineView(): Gtk::Grid() {
		add_css_class("lineview");
		set_column_spacing(5);
	}

	LineView & LineView::operator+=(const std::string &text) {
		std::stringstream ss;
		time_t now = std::time(nullptr);
		tm *times = std::localtime(&now);
		ss << "[" << std::setfill('0') << std::setw(2) << times->tm_hour << ":" << std::setw(2) << times->tm_min << ":"
		   << std::setw(2) << times->tm_sec << "]";
		auto &vector = widgets.emplace_back();
		vector.reserve(2);
		auto &timestamp = *vector.emplace_back(std::make_unique<Gtk::Label>(ss.str(), Gtk::Align::START));
		timestamp.add_css_class("timestamp");
		attach(timestamp, 0, row);
		attach(*vector.emplace_back(std::make_unique<Gtk::Label>(text, Gtk::Align::START)), 1, row);
		++row;
		return *this;
	}
}
