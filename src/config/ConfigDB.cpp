#include <fstream>
#include <locale>
#include <sstream>
#include <stdexcept>

#include <cstdlib>

#include "config/ConfigDB.h"
#include "config/Defaults.h"
#include "core/Util.h"

#include "lib/formicine/futil.h"

namespace Reden {

// Private static methods


	bool ConfigDB::parseBool(const Glib::ustring &str) {
		return str == "true" || str == "on" || str == "yes";
	}


// Private instance methods


	void ConfigDB::ensureKnown(const Glib::ustring &group, const Glib::ustring &key) const {
		if (!allowUnknown && !keyKnown(group, key))
			throw std::invalid_argument("Unknown group+key pair");
	}


// Public static methods


	Glib::ustring ConfigDB::escape(const Value &value) {
		if (std::holds_alternative<long>(value))
			return std::to_string(std::get<long>(value));
		if (std::holds_alternative<double>(value))
			return std::to_string(std::get<double>(value));
		if (std::holds_alternative<bool>(value))
			return std::get<bool>(value)? "true" : "false";
		if (std::holds_alternative<Glib::ustring>(value))
			return Util::escape(std::get<Glib::ustring>(value));
		throw std::invalid_argument("Configuration value holds invalid type");
	}

	std::pair<Glib::ustring, long> ConfigDB::parseLongLine(const Glib::ustring &str) {
		Glib::ustring key, value;
		std::tie(key, value) = parseKVPair(str);

		const char *value_cstr = value.c_str();
		char *end;
		long parsed = strtol(value_cstr, &end, 10);
		if (end != value_cstr + value.size())
			throw std::invalid_argument("Invalid value in key-value pair; expected a long");

		return {key, parsed};
	}

	std::pair<Glib::ustring, double> ConfigDB::parseDoubleLine(const Glib::ustring &str) {
		Glib::ustring key, value;
		std::tie(key, value) = parseKVPair(str);

		if (value == ".")
			return {key, 0};

		size_t idx;
		double parsed = std::stod(value, &idx);
		if (idx != value.length())
			throw std::invalid_argument("Invalid value in key-value pair; expected a double");

		return {key, parsed};
	}

	std::pair<Glib::ustring, bool> ConfigDB::parseBoolLine(const Glib::ustring &str) {
		Glib::ustring key, value;
		std::tie(key, value) = parseKVPair(str);

		if (value == ".")
			return {key, 0};

		size_t idx;
		bool parsed = std::stod(value, &idx);
		if (idx != value.length())
			throw std::invalid_argument("Invalid value in key-value pair; expected a bool");

		return {key, parsed};
	}

	std::pair<Glib::ustring, Glib::ustring> ConfigDB::parsePair(const Glib::ustring &str) {
		size_t period = str.find('.');
		const bool at_edge = period == Glib::ustring::npos || period == 0 || period == str.length() - 1;
		if (at_edge || period != str.find_last_of("."))
			throw std::invalid_argument("Invalid group+key pair");
		return {str.substr(0, period), str.substr(period + 1)};
	}

	ValueType ConfigDB::getValueType(Glib::ustring val) noexcept {
		formicine::util::trim(val);

		if (val.empty())
			return ValueType::String;

		if (val.find_first_not_of("0123456789") == Glib::ustring::npos)
			return ValueType::Long;

		if (val.find_first_not_of("0123456789.") == Glib::ustring::npos) {
			// Don't allow multiple periods in the string.
			if (val.find('.') != val.find_last_of("."))
				return ValueType::Invalid;
			return ValueType::Double;
		}

		if (val == "true" || val == "false" || val == "on" || val == "off" || val == "yes" || val == "no")
			return ValueType::Bool;

		if (2 <= val.size() && val[0] == '"' && val[val.size() - 1] == '"')
			return ValueType::String;

		return ValueType::Invalid;
	}


// Public instance methods


	Value & ConfigDB::get(const Glib::ustring &group, const Glib::ustring &key) {
		ensureKnown(group, key);

		if (hasKey(group, key))
			return db.at(group).at(key);

		if (keyKnown(group, key))
			return registered.at(group + "." + key).defaultValue;

		throw std::out_of_range("No value for group+key pair");
	}

	Value & ConfigDB::getPair(const std::pair<Glib::ustring, Glib::ustring> &pair) {
		return get(pair.first, pair.second);
	}

	bool ConfigDB::insert(const Glib::ustring &group, const Glib::ustring &key, const Value &value, bool save) {
		ensureKnown(group, key);

		SubMap &sub = db[group];
		bool overwritten = false;

		const auto iter = registered.find(group + "." + key);
		const bool is_registered = iter != registered.end();
		if (is_registered) {
			ValidationResult result = iter->second.validate(value);
			if (result != ValidationResult::Valid)
				throw ValidationFailure(result);
		}

		if (sub.count(key) > 0) {
			sub.erase(key);
			overwritten = true;
		}

		sub.insert({key, value});

		if (save)
			writeDB();

		if (is_registered)
			iter->second.apply(*this, value);

		return overwritten;
	}

	bool ConfigDB::insertAny(const Glib::ustring &group, const Glib::ustring &key, const Glib::ustring &value,
	                         bool save) {
		const ValueType type = ConfigDB::getValueType(value);
		switch (type) {
			case ValueType::Long:   return insert(group, key, {strtol(value.c_str(), nullptr, 10)}, save);
			case ValueType::Double: return insert(group, key, {std::stod(value)}, save);
			case ValueType::Bool:   return insert(group, key, {parseBool(value)}, save);
			case ValueType::String: return insert(group, key, {ConfigDB::parseString(value)}, save);
			default:
				throw std::invalid_argument("Invalid value type");
		}
	}

	bool ConfigDB::remove(const Glib::ustring &group, const Glib::ustring &key, bool apply_default, bool save) {
		if (!hasKey(group, key))
			return false;

		db[group].erase(key);

		if (apply_default) {
			const Glib::ustring combined {group + "." + key};
			if (registered.count(combined) == 1)
				registered.at(combined).apply(*this);
		}

		if (save)
			writeDB();

		return true;
	}

	std::pair<Glib::ustring, Glib::ustring> ConfigDB::applyLine(const Glib::ustring &line) {
		if (!line.empty() && line[0] == '#')
			return {"", ""};

		Glib::ustring group, key, gk, value;
		std::tie(gk, value) = parseKVPair(line);
		std::tie(group, key) = parsePair(gk);
		insertAny(group, key, value);
		return {group + "." + key, value};
	}

	void ConfigDB::applyAll(bool with_defaults) {
		for (auto &pair: registered) {
			Glib::ustring group, key;
			std::tie(group, key) = parsePair(pair.first);
			if (hasKey(group, key)) {
				pair.second.apply(*this, get(group, key));
			} else if (with_defaults) {
				pair.second.apply(*this);
			}
		}
	}

	bool ConfigDB::hasGroup(const Glib::ustring &group) const {
		return db.count(group) > 0;
	}

	bool ConfigDB::hasKey(const Glib::ustring &group, const Glib::ustring &key) const {
		return hasGroup(group) && db.at(group).count(key) > 0;
	}

	bool ConfigDB::keyKnown(const Glib::ustring &group, const Glib::ustring &key) const {
		return registered.count(group + "." + key) > 0;
	}

	ssize_t ConfigDB::keyCount(const Glib::ustring &group) const {
		return hasGroup(group)? db.at(group).size() : -1;
	}

	ConfigDB::GroupMap ConfigDB::withDefaults() const {
		GroupMap copy {db};
		for (const auto &gpair: registered) {
			const DefaultConfigKey &def = gpair.second;

			Glib::ustring group, key;
			const Glib::ustring &combined = gpair.first;
			std::tie(group, key) = parsePair(combined);

			copy[group].insert({key, def.defaultValue});
		}

		return copy;
	}

	ConfigDB::operator Glib::ustring() const {
		std::ostringstream out;
		for (const auto &gpair: db) {
			const Glib::ustring &group = gpair.first;
			const SubMap &sub = gpair.second;

			for (const auto &spair: sub) {
				const Glib::ustring &key = spair.first;
				const Value &value = spair.second;
				out << group << "." << key << "=" << escape(value) << "\n";
			}
		}

		return out.str();
	}
}
