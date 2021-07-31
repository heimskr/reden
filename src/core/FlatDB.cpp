#include "core/FlatDB.h"
#include "core/Util.h"

namespace Reden {

// Protected static methods


	std::filesystem::path FlatDB::getDBPath(const std::string &dbname, const std::string &dirname) {
		if (!dirname.empty() && dirname.front() == '/')
			return std::filesystem::path(dirname) / dbname;
		return Util::getHome() / dirname / dbname;
	}

	std::pair<Glib::ustring, Glib::ustring> FlatDB::parseKVPair(const Glib::ustring &str) {
		if (str.empty())
			throw std::invalid_argument("Can't parse empty string as key-value pair");

		const size_t equals = str.find('=');

		if (equals == Glib::ustring::npos)
			throw std::invalid_argument("No equals sign found in key-value pair");

		if (equals == 0 || equals == str.find_first_not_of(" "))
			throw std::invalid_argument("Empty key in key-value pair");

		Glib::ustring key = str.substr(0, equals);
		formicine::util::trim(key);

		for (char ch: key) {
			if (!std::isalnum(ch) && ch != '.' && ch != '_')
				throw std::invalid_argument("Key isn't alphanumeric, '.' or '_' in key-value pair");
		}

		return {key, formicine::util::trim(str.substr(equals + 1))};
	}

	Glib::ustring FlatDB::parseString(Glib::ustring value) {
		value = formicine::util::trim(std::string(value));
		const size_t vlength = value.length();

		// Special case: an empty value represents an empty string, same as a pair of double quotes.
		if (vlength == 0)
			return "";

		if (vlength < 2)
			throw std::invalid_argument("Invalid length of string value");

		if (value[0] != '"' || value[value.size() - 1] != '"')
			throw std::invalid_argument("Invalid quote placement in string value");

		return Util::unescape(value.substr(1, vlength - 2));
	}

	std::pair<Glib::ustring, Glib::ustring> FlatDB::parseStringLine(const Glib::ustring &str) {
		auto [key, value] = parseKVPair(str);
		return {key, parseString(value)};
	}


// Protected instance methods


	void FlatDB::writeDB() {
		std::ofstream out {filepath};
		out << Glib::ustring(*this);
		out.close();
	}

	void FlatDB::readDB(bool apply, bool clear) {
		if (clear)
			clearAll();

		std::ifstream in {filepath};
		std::string line;
		while (std::getline(in, line))
			applyLine(line);

		if (apply)
			applyAll();
	}


// Public instance methods


	void FlatDB::setPath(const Glib::ustring &dbname, bool apply, const Glib::ustring &dirname) {
		ensureDB(dbname, dirname);
		filepath = getDBPath(dbname, dirname);
		readDB(apply);
	}

	void FlatDB::readIfEmpty(const Glib::ustring &dbname, bool apply, const Glib::ustring &dirname) {
		if (filepath.empty())
			setPath(dbname, apply, dirname);
		else if (empty())
			readDB(apply);
	}


// Public static methods


	bool FlatDB::ensureDirectory(const Glib::ustring &name) {
		if (name.empty())
			throw std::invalid_argument("Directory path is empty");
		std::filesystem::path config_path =
			name[0] == '/'? std::filesystem::path(name) : Util::getHome() / std::string(name);
		if (!std::filesystem::exists(config_path)) {
			if (!std::filesystem::create_directories(config_path))
				throw std::runtime_error("Couldn't create directories");
			return true;
		}

		return false;
	}

	bool FlatDB::ensureDB(const Glib::ustring &dbname, const Glib::ustring &dirname) {
		ensureDirectory(dirname);
		std::filesystem::path db_path = getDBPath(dbname, dirname);

		bool created = false;
		if (!std::filesystem::exists(db_path)) {
			std::ofstream(db_path).close();
			created = true;
		}

		return created;
	}
}
