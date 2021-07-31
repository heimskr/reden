#include <iostream>

#include "App.h"
#include "tests/Tests.h"

int main(int argc, char *argv[]) {
	if (argc == 2 && strcmp(argv[1], "irc2pango") == 0) {
		return Reden::Tests::irc2pangoTests()? 0 : 1;
	} else {
		Glib::RefPtr<Reden::App> app = Reden::App::create();
		return app->run(argc, argv);
	}

	return 0;
}
