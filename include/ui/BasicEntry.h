#pragma once

#include <gtkmm.h>

namespace Reden {
	class BasicEntry: public Gtk::Entry {
		public:
			BasicEntry();
			sigc::signal<void()> signal_activate() const { return signal_activate_; }

		private:
			sigc::signal<void()> signal_activate_;
	};
}
