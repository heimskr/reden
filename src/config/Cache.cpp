#include "config/Cache.h"

namespace Reden {
	const Glib::ustring & ConfigCache::getString(const Glib::ustring &key) {
		if (registered.empty())
			registerDefaults();
		return std::get<Glib::ustring>(registered.at(key).defaultValue);
	}

	bool ConfigCache::getBool(const Glib::ustring &key) {
		if (registered.empty())
			registerDefaults();
		return std::get<bool>(registered.at(key).defaultValue);
	}

	long ConfigCache::getLong(const Glib::ustring &key) {
		if (registered.empty())
			registerDefaults();
		return std::get<long>(registered.at(key).defaultValue);
	}
}