#include <iostream>

#include "ui/RedenWindow.h"

namespace Reden {
	RedenWindow::RedenWindow(BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &builder_):
	Gtk::ApplicationWindow(cobject), builder(builder_) {
		header = builder->get_widget<Gtk::HeaderBar>("headerbar");
		set_titlebar(*header);

		add_action("connect", Gio::ActionMap::ActivateSlot([this] {
			std::cout << "connect\n";
		}));
	}

	RedenWindow * RedenWindow::create() {
		auto builder = Gtk::Builder::create_from_resource("/com/heimskr/reden/window.ui");
		auto window = Gtk::Builder::get_widget_derived<RedenWindow>(builder, "reden_window");
		if (!window)
			throw std::runtime_error("No \"reden_window\" object in window.ui");
		return window;
	}
}