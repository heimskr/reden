#pragma once

#include <gtkmm.h>
#include <utility>

namespace Reden {
	enum class KeyCode: guint {
		Tab = 23
	};

	struct Key {
		guint code;
		Gdk::ModifierType modifiers;

		Key(): Key(0) {}
		Key(guint code_): Key(code_, static_cast<Gdk::ModifierType>(0)) {}
		Key(guint, Gdk::ModifierType);

		bool operator==(guint) const;
		bool operator==(KeyCode) const;
	};
}
