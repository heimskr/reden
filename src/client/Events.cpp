#include "core/Client.h"
#include "ui/RedenWindow.h"

#include "pingpong/events/Event.h"
#include "pingpong/events/ServerStatus.h"

namespace Reden {
	void Client::addEvents() {
		PingPong::Events::listen<PingPong::ServerStatusEvent>([&](PingPong::ServerStatusEvent *ev) {
			callInQueue(ev->server, ev->server->getStatus());

			if (ev->server->getStatus() == PingPong::Server::Stage::Ready)
				window.box.addStatus("Connected to " + ev->server->id + ".");
		});
	}

	void Client::callInQueue(PingPong::Server *server, PingPong::Server::Stage stage) {
		if (serverStatusQueue.count(server) == 0)
			return;

		std::list<QueuePair> &list = serverStatusQueue.at(server);
		for (auto iter = list.begin(); iter != list.end();) {
			const PingPong::Server::Stage requested_stage = iter->first;
			const QueueFn &fn = iter->second;

			if (requested_stage == stage) {
				fn();
				list.erase(iter++);
			} else
				++iter;
		}
	}

	void Client::waitForServer(PingPong::Server *server, PingPong::Server::Stage stage, const Client::QueueFn &fn) {
		if (server->getStatus() == stage)
			fn();
		else
			serverStatusQueue[server].push_back({stage, fn});
	}
}
