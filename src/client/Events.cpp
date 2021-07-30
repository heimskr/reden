#include "core/Client.h"
#include "ui/RedenWindow.h"

#include "pingpong/events/Join.h"
#include "pingpong/events/Mode.h"
#include "pingpong/events/NamesUpdated.h"
#include "pingpong/events/Part.h"
#include "pingpong/events/Privmsg.h"
#include "pingpong/events/Raw.h"
#include "pingpong/events/ServerStatus.h"
#include "pingpong/events/Topic.h"

namespace Reden {
	void Client::addEvents() {
		PingPong::Events::listen<PingPong::JoinEvent>([this](PingPong::JoinEvent *ev) {
			const bool self = ev->who->isSelf();
			auto channel = ev->channel;
			auto name = ev->who->name;
			window.queue([this, channel, name, self] {
				if (self)
					window.box.addChannel(channel.get(), true);
				window.box[channel].joined(name, channel->name);
				window.box.updateChannel(*channel);
			});
		});

		PingPong::Events::listen<PingPong::ModeEvent>([this](PingPong::ModeEvent *ev) {
			auto who = ev->who;
			auto modeset = ev->modeSet;
			if (auto channel = ev->getChannel(ev->server))
				window.queue([this, channel, who, modeset] {
					window.box[channel].mode(channel, who, modeset);
					window.box.updateChannel(*channel);
				});
		});

		PingPong::Events::listen<PingPong::NamesUpdatedEvent>([this](PingPong::NamesUpdatedEvent *ev) {
			auto channel = ev->channel;
			window.queue([this, channel] {
				window.box.updateChannel(*channel);
			});
		});

		PingPong::Events::listen<PingPong::PartEvent>([this](PingPong::PartEvent *ev) {
			if (ev->who->isSelf()) {
				auto channel = ev->channel;
				window.queue([this, channel] {
					window.box.eraseChannel(channel.get());
				});
			}
		});

		PingPong::Events::listen<PingPong::PrivmsgEvent>([this](PingPong::PrivmsgEvent *ev) {
			if (ev->isChannel()) {
				const std::string content = ev->content;
				auto channel = ev->server->getChannel(ev->where, true);
				const std::string name = channel->withHat(ev->speaker);
				window.queue([this, content, channel, name] {
					window.box[channel].addMessage(name, content);
				});
			}
		});

		PingPong::Events::listen<PingPong::RawInEvent>([this](PingPong::RawInEvent *ev) {
			auto server = ev->server;
			auto raw = ev->rawIn;
			while (!raw.empty() && (raw.back() == '\r' || raw.back() == '\n'))
				raw.pop_back();
			window.queue([this, server, raw] {
				window.box.addServer(server, false);
				window.box[server] += "<< " + raw;
			});
		});

		PingPong::Events::listen<PingPong::RawOutEvent>([this](PingPong::RawOutEvent *ev) {
			auto server = ev->server;
			auto raw = ev->rawOut;
			while (!raw.empty() && (raw.back() == '\r' || raw.back() == '\n'))
				raw.pop_back();
			window.queue([this, server, raw] {
				window.box.addServer(server, false);
				window.box[server] += ">> " + raw;
			});
		});

		PingPong::Events::listen<PingPong::ServerStatusEvent>([this](PingPong::ServerStatusEvent *ev) {
			callInQueue(ev->server, ev->server->getStatus());

			if (ev->server->getStatus() == PingPong::Server::Stage::Ready)
				window.box.addStatus("Connected to " + ev->server->id + ".");

			switch (ev->server->getStatus()) {
				case PingPong::Server::Stage::Ready: {
					auto server = ev->server;
					window.queue([this, server] {
						window.box.addServer(server, true);
						window.box.addStatus("Connected to " + server->id + " (" + server->hostname + ":"
							+ std::to_string(server->port) + ")");
					});
					break;
				}
				case PingPong::Server::Stage::Dead: {
					auto server = ev->server;
					window.queue([this, server] {
						window.box.eraseServer(server);
					});
					break;
				}
				default:
					break;
			}
		});

		PingPong::Events::listen<PingPong::TopicEvent>([this](PingPong::TopicEvent *ev) {
			auto channel = ev->channel;
			auto who = ev->who;
			window.queue([this, channel, who] {
				window.box.setTopic(channel.get(), std::string(channel->topic));
				window.box[channel].topicChanged(channel, who, std::string(channel->topic));
			});
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
		std::cout << "waitForServer(server=" << server->statusString() << ", stage=" << static_cast<int>(stage) << ")\n";
		if (server->getStatus() == stage)
			fn();
		else
			serverStatusQueue[server].push_back({stage, fn});
	}
}
