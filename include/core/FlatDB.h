#pragma once

#include <gtkmm.h>
#include <filesystem>
#include <mutex>
#include <string>
#include <utility>

#include "core/Options.h"

namespace Reden {
	class FlatDB {
		private:
			std::recursive_mutex mutex;

		protected:
			/** The path where the database will be read from and written to. */
			std::filesystem::path filepath;

			/** Given a data directory name and a database name, this returns the full path of the database. */
			static std::filesystem::path getDBPath(const std::string &dbname,
			                                       const std::string &dirname = DEFAULT_DATA_DIR);

			/** Attempts to parse a keyvalue pair of the form /^(\w+)=(.+)$/. */
			static std::pair<Glib::ustring, Glib::ustring> parseKVPair(const Glib::ustring &);

			/** Attempts to parse a string from a key-value pair. */
			static Glib::ustring parseString(Glib::ustring);

			/** Attempts to parse a configuration line of the form /^\w+\s*=\s*("[^\\\n\r\t\0"]*")?$/. */
			static std::pair<Glib::ustring, Glib::ustring> parseStringLine(const Glib::ustring &);

		public:
			virtual ~FlatDB() {}

			std::unique_lock<std::recursive_mutex> lockDB() { return std::unique_lock(mutex); }

			/** Applies a line of read input. */
			virtual std::pair<Glib::ustring, Glib::ustring> applyLine(const Glib::ustring &) = 0;

			/** Applies all database items. Useful for the configuration database, which has applicators, but less
			 *  useful for the alias database which just reads items and doesn't immediately act on them. */
			virtual void applyAll() {}

			/** Clears all database items. */
			virtual void clearAll() = 0;

			/** Returns whether the database is empty. */
			virtual bool empty() const = 0;

			/** Sets the cached database path and replaces the cached database with the one stored at the path. */
			void setPath(const Glib::ustring &dbname, bool apply = true,
			             const Glib::ustring &dirname = DEFAULT_DATA_DIR);

			/** Reads the database from the filesystem if the in-memory copy is empty. */
			void readIfEmpty(const Glib::ustring &dbname, bool apply = true,
			                 const Glib::ustring &dirname = DEFAULT_DATA_DIR);

			/** Writes the database to the cached file path. */
			void writeDB();

			/** Reads the database from the cached file path. */
			void readDB(bool apply = true, bool clear = true);

			/** Stringifies the database. */
			virtual operator Glib::ustring() = 0;

			/** Creates a config directory in the user's home directory if one doesn't
			 *  already exist. Returns true if the directory had to be created. */
			static bool ensureDirectory(const Glib::ustring &name = DEFAULT_DATA_DIR);

			/** Ensures the config directory exists and creates a blank config database inside it if
			 *  one doesn't already exist. Returns true if the config database had to be created. */
			static bool ensureDB(const Glib::ustring &dbname, const Glib::ustring &dirname = DEFAULT_DATA_DIR);
	};
}
