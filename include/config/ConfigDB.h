#pragma once

#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <unordered_set>
#include <utility>
#include <variant>

#include "config/KeyConfig.h"
#include "config/Validation.h"
#include "core/FlatDB.h"
#include "core/Ustring.h"

namespace Reden {
	class Client;

	enum class ValueType {Invalid, Long, Double, Bool, String};
	using Value = std::variant<long, double, bool, Glib::ustring>;

	/** Represents a configuration database. */
	class ConfigDB: public FlatDB {
		public:
			using   SubMap = std::map<Glib::ustring, Value>;
			using GroupMap = std::map<Glib::ustring, SubMap>;

		private:
			/** The in-memory copy of the config database. */
			GroupMap db;

			/** Whether to allow unknown group+key combinations to be inserted into the database. */
			bool allowUnknown;

			static bool parseBool(const Glib::ustring &str);

			/** Throws a std::invalid_argument exception if a group+key pair is unknown and unknown group+key pairs
			 *  aren't allowed. */
			void ensureKnown(const Glib::ustring &group, const Glib::ustring &key) const noexcept(false);

		public:
			Client &parent;

			ConfigDB() = delete;
			ConfigDB(const ConfigDB &) = delete;
			ConfigDB(Client &parent_, bool allow_unknown): allowUnknown(allow_unknown), parent(parent_) {}

			ConfigDB operator=(const ConfigDB &) = delete;

			virtual ~ConfigDB() override {}

			static Glib::ustring escape(const Value &);

			/** Attempts to parse a configuration line of the form /^\w+\s*=\s*\d+$/. */
			static std::pair<Glib::ustring, long> parseLongLine(const Glib::ustring &);

			/** Attempts to parse a configuration line of the form /^\w+\s*=\s*\d+\.\d*$/. */
			static std::pair<Glib::ustring, double> parseDoubleLine(const Glib::ustring &);

			/** Attempts to parse a configuration line of the form /^\w+\s*=\s*(true|false|on|off|yes|no)$/. */
			static std::pair<Glib::ustring, bool> parseBoolLine(const Glib::ustring &);

			/** Attempts to split a "group.key" pair. Throws std::invalid_argument if there isn't exactly one period in
			 *  the string or if the area before or after the period contains nothing. */
			static std::pair<Glib::ustring, Glib::ustring> parsePair(const Glib::ustring &);

			/** Checks a value and returns its type. */
			static ValueType getValueType(Glib::ustring) noexcept;

			/** Inserts a value into the config database. Returns true if a preexisting value was overwritten. */
			bool insert(const Glib::ustring &group, const Glib::ustring &key, const Value &, bool save = true);

			/** Inserts a value into the config database. Returns true if a preexisting value was overwritten. */
			bool insertAny(const Glib::ustring &group, const Glib::ustring &key, const Glib::ustring &,
			               bool save = true);

			/** Removes a value from the config database and optionally applies the default value for the key if one has
			 *  been registered. Returns true if a value was present and removed, or false if no match was found. */
			bool remove(const Glib::ustring &group, const Glib::ustring &key, bool apply_default = true,
			            bool save = true);

			/** Applies all settings, optionally with default settings where not overridden. */
			void applyAll(bool with_defaults);

			virtual std::pair<Glib::ustring, Glib::ustring> applyLine(const Glib::ustring &) override;

			virtual void applyAll() override { applyAll(true); }

			virtual void clearAll() override { db.clear(); }

			virtual bool empty() const override { return db.empty(); }

			/** Returns a value from the config database. If an unknown group+key pair is given and not present in the
			 *  database, a std::out_of_range exception is thrown. */
			Value & get(const Glib::ustring &group, const Glib::ustring &key);

			Value & getPair(const std::pair<Glib::ustring, Glib::ustring> &pair);

			Glib::ustring & getString(const Glib::ustring &group, const Glib::ustring &key);
			long & getLong(const Glib::ustring &group, const Glib::ustring &key);
			double & getDouble(const Glib::ustring &group, const Glib::ustring &key);
			bool & getBool(const Glib::ustring &group, const Glib::ustring &key);

			/** Returns whether a group name is present in the config database. */
			bool hasGroup(const Glib::ustring &) const;

			/** Returns whether a key name is present within a given group in the config database. */
			bool hasKey(const Glib::ustring &group, const Glib::ustring &key) const;

			/** Returns whether a group+key pair has been registered. */
			bool keyKnown(const Glib::ustring &group, const Glib::ustring &key) const;

			/** Returns the number of keys present under a group. If the group doesn't exist in the config database, the
			 *  function returns -1. */
			ssize_t keyCount(const Glib::ustring &group) const;

			/** Returns all the known keys for a group, including any defaults. Throws std::runtime_error if the group
			 *  doesn't exist. */
			std::unordered_set<Glib::ustring> allKeys(const Glib::ustring &group) const;

			/** Returns all the known group names, including any defaults. */
			std::unordered_set<Glib::ustring> allGroups() const;

			/** Returns a copy of the config database with all default keys filled in if not already present. */
			GroupMap withDefaults();

			/** Stringifies the config database. */
			operator Glib::ustring() override;

			GroupMap::iterator begin() { return db.begin(); }
			GroupMap::iterator end() { return db.end(); }
	};
}
