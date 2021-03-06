#include <iostream>

#include "App.h"

namespace Reden {
	App::App(): Gtk::Application("com.heimskr.reden", Gio::Application::Flags::NONE) {}

	Glib::RefPtr<App> App::create() {
		return Glib::make_refptr_for_instance<App>(new App());
	}

	void App::on_startup() {
		Gtk::Application::on_startup();
		set_accel_for_action("win.connect", "<Ctrl>o");
		set_accel_for_action("win.disconnect", "<Ctrl>d");
		set_accel_for_action("win.reload-css", "<Ctrl>r");
	}

	void App::on_activate() {
		try {
			auto *window = create_window();
			window->present();
		} catch (const Glib::Error &err) {
			std::cerr << "App::on_activate(): " << err.what() << std::endl;
		} catch (const std::exception &err) {
			std::cerr << "App::on_activate(): " << err.what() << std::endl;
		}
	}

	RedenWindow * App::create_window() {
		RedenWindow *window = RedenWindow::create();
		add_window(*window);
		window->signal_hide().connect(sigc::bind(sigc::mem_fun(*this, &App::on_hide_window), window));
		return window;
	}

	void App::on_hide_window(Gtk::Window *window) {
		delete window;
	}

	const char * App::getText(const std::string &path, gsize &size) {
		return reinterpret_cast<const char *>(Gio::Resource::lookup_data_global(path)->get_data(size));
	}

	const char * App::getText(const std::string &path) {
		gsize size;
		return getText(path, size);
	}
}
