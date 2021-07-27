#include <iostream>

#include "App.h"

int main(int argc, char *argv[]) {
	Glib::RefPtr<Reden::App> app = Reden::App::create();
	return app->run(argc, argv);
}
