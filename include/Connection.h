#pragma once

#include <map>
#include <string>

namespace Reden {
	struct Connection {
		std::string host, name;
		std::vector<std::string> channels;
		std::string activeChannel;
		int port = 0;

		Connection() {}
		Connection(const std::string &host_, const std::string &name_, int port_):
			host(host_), name(name_), port(port_) {}
		Connection(const std::string &host_, int port_):
			Connection(host_, host_, port_) {}

		operator bool() const { return port != 0 && !host.empty(); }
	};

	struct ConnectionMap: public std::map<std::string, Connection> {
		using std::map<std::string, Connection>::map;
		std::string active;
	};
}
