#pragma once

#include <gtkmm.h>

namespace Reden {
	/** Converts an IRC message to pango markup. */
	Glib::ustring irc2pango(Glib::ustring);
}
