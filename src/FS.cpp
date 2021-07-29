#include <fstream>
#include <sys/stat.h>

#include "FS.h"

namespace FS {
	bool fileExists(const char *path) {
		struct stat buffer;
		return stat(path, &buffer) == 0 && (buffer.st_mode & S_IFDIR) == 0;
	}

	bool fileExists(const std::string &path) {
		return fileExists(path.c_str());
	}

	bool dirExists(const char *path) {
		struct stat buffer;
		return stat(path, &buffer) == 0 && (buffer.st_mode & S_IFDIR) == S_IFDIR;
	}

	bool dirExists(const std::string &path) {
		return dirExists(path.c_str());
	}

	std::string readFile(const char *path) {
		std::ifstream stream(path);
		stream.seekg(0, std::ios::end);
		const size_t size = stream.tellg();
		std::string buffer(size, ' ');
		stream.seekg(0);
		stream.read(&buffer[0], size);
		stream.close();
		return buffer;
	}

	std::string readFile(const std::string &path) {
		return readFile(path.c_str());
	}

	void writeFile(const char *path, const std::string &text) {
		std::ofstream stream(path);
		stream << text;
		stream.close();
	}

	void writeFile(const std::string &path, const std::string &text) {
		writeFile(path.c_str(), text);
	}
}
