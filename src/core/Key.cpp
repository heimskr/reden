#include "core/Key.h"

namespace Reden {
	Key::Key(guint code_, Gdk::ModifierType modifiers_): code(code_), modifiers(modifiers_) {}

	bool Key::operator==(guint code_) const {
		return code == code_;
	}

	bool Key::operator==(KeyCode code_) const {
		return code == static_cast<guint>(code_);
	}
}
