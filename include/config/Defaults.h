#pragma once

#include <gtkmm.h>
#include <functional>
#include <map>
#include <string>

#include "config/ConfigDB.h"

namespace Reden {
	using Applicator = std::function<void(ConfigDB &, const Value &)>;
	using Validator  = std::function<ValidationResult(const Value &)>;

	struct DefaultConfigKey {
		Glib::ustring name, description;
		Value defaultValue;
		Validator validator;
		Applicator applicator;

		DefaultConfigKey(const Glib::ustring &name_, const Value &default_value, const Validator &validator_,
		const Applicator &applicator_, const Glib::ustring &description_ = ""):
			name(name_), description(description_), defaultValue(default_value), validator(validator_),
			applicator(applicator_) {}

		DefaultConfigKey(const Glib::ustring &name_, const Value &default_value, const Validator &validator_,
		const Glib::ustring &description_ = ""):
			DefaultConfigKey(name_, default_value, validator_, {}, description_) {}

		DefaultConfigKey(const Glib::ustring &name_, const Value &default_value, const Applicator &applicator_,
		const Glib::ustring &description_ = ""):
			DefaultConfigKey(name_, default_value, {}, applicator_, description_) {}

		DefaultConfigKey(const Glib::ustring &name_, const Value &default_value,
		                 const Glib::ustring &description_ = ""):
			DefaultConfigKey(name_, default_value, {}, {}, description_) {}

		ValidationResult validate(const Value &val) const {
			return validator? validator(val) : ValidationResult::Valid;
		}

		void apply(ConfigDB &db, const Value &new_value) {
			if (applicator)
				applicator(db, new_value);
		}

		void apply(ConfigDB &db) { apply(db, defaultValue); }
	};

	using RegisteredMap = std::map<Glib::ustring, DefaultConfigKey>;

	// Oh no! Macros!

#define CACHE_BOOL(name)   [](ConfigDB &db, const Value &new_val) { db.parent.cache.name = std::get<bool>(new_val); }
#define CACHE_LONG(name)   [](ConfigDB &db, const Value &new_val) { db.parent.cache.name = std::get<long>(new_val); }
#define CACHE_STRING(name) [](ConfigDB &db, const Value &new_val) { db.parent.cache.name = \
                                                                        std::get<Glib::ustring>(new_val); }

	/** Attempts to register a key. If the key already exists, the function simply returns false; otherwise, it
	 *  registers the key and returns true. */
	bool registerKey(const Glib::ustring &group, const Glib::ustring &key, const Value &default_val,
	                 const Validator & = {}, const Applicator & = {}, const Glib::ustring &description = "");

	/** Attempts to unregister a key. Returns true if the key existed and was removed. */
	bool unregister(const Glib::ustring &group, const Glib::ustring &key);

	/** Runs the applicators of all registered defaults with their default values. */
	void applyDefaults(ConfigDB &db);

	/** Returns a vector of the names of all default keys whose full name or key name begins with a given string. */
	std::vector<Glib::ustring> startsWith(const Glib::ustring &);

	/** Registers the standard Spjalla configuration keys. */
	void registerDefaults();
	void registerAppearance();
	void registerFormat();

	extern RegisteredMap registered;

	ValidationResult validateLong(const Value &);
	ValidationResult validateNonnegative(const Value &);
	ValidationResult validateUint32(const Value &);
	ValidationResult validateInt32nn(const Value &);
	ValidationResult validateString(const Value &);
	ValidationResult validateBool(const Value &);
}
