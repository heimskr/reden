#include "core/Client.h"
#include "ui/RedenWindow.h"

#include "pingpong/events/Join.h"
#include "pingpong/events/Mode.h"
#include "pingpong/events/NamesUpdated.h"
#include "pingpong/events/Part.h"
#include "pingpong/events/Privmsg.h"
#include "pingpong/events/Quit.h"
#include "pingpong/events/Raw.h"
#include "pingpong/events/ServerStatus.h"
#include "pingpong/events/Topic.h"
#include "pingpong/events/TopicUpdated.h"

namespace Reden {
	void Client::addEvents() {
		PingPong::Events::listen<PingPong::JoinEvent>([this](PingPong::JoinEvent *ev) {
			const bool self = ev->who->isSelf();
			auto channel = ev->channel;
			auto name = ev->who->name;
			window.queue([this, channel, name, self] {
				if (self)
					window.box.add(channel.get(), true);
				window.box[channel].joined(name, channel->name);
				window.box.updateChannel(*channel);
			});
		});

		PingPong::Events::listen<PingPong::ModeEvent>([this](PingPong::ModeEvent *ev) {
			const auto &who = ev->who;
			const auto &modeset = ev->modeSet;
			if (auto channel = ev->getChannel(ev->server))
				window.queue([this, channel, who, modeset] {
					window.box[channel].mode(channel, who, modeset);
					window.box.updateChannel(*channel);
				});
		});

		PingPong::Events::listen<PingPong::NamesUpdatedEvent>([this](PingPong::NamesUpdatedEvent *ev) {
			const auto &channel = ev->channel;
			window.queue([this, channel] {
				window.box.updateChannel(*channel);
			});
		});

		PingPong::Events::listen<PingPong::PartEvent>([this](PingPong::PartEvent *ev) {
			const auto &channel = ev->channel;
			if (ev->who->isSelf()) {
				window.queue([this, channel] {
					window.box.erase(channel.get());
				});
			} else {
				const auto &name = ev->who->name;
				const auto &reason = ev->content;
				window.queue([this, channel, name, reason] {
					window.box.add(channel.get(), false);
					window.box[channel].parted(name, channel->name, reason);
				});
			}
		});

		PingPong::Events::listen<PingPong::PrivmsgEvent>([this](PingPong::PrivmsgEvent *ev) {
			std::string content = ev->content;
			if (ev->isChannel()) {
				const auto channel = ev->server->getChannel(ev->where, true);
				const bool default_behavior = cache.interfacePlaybackMode == "default";

				if (ev->speaker->name == "***") {
					if (ev->content == "Buffer Playback...") {
						playbackModeChannels.insert(channel);
						if (!default_behavior)
							return;
					} else if (ev->content == "Playback Complete.") {
						playbackModeChannels.erase(channel);
						if (!default_behavior)
							return;
					}
				}

				const std::string name = channel->withHat(ev->speaker);
				const bool is_self = ev->speaker->isSelf();

				if (playbackModeChannels.count(channel) != 0 && !default_behavior) {
					if (cache.interfacePlaybackMode == "ignore")
						return;

					if (content.find("\x01" "ACTION ") == 0) {
						content.erase(0, 8);
						content.insert(11, "\x01" "ACTION ");
					}

					if (content.size() < 11 || content[0] != '[' || content[9] != ']' || content[10] != ' ')
						// 11 = strlen("[xx:xx:xx] ")
						throw std::runtime_error("Invalid playback string: \"" + content + "\"");
					const std::string hour_str = content.substr(1, 2),
					                minute_str = content.substr(4, 2),
					                second_str = content.substr(7, 2);
					content.erase(0, 11);
					long hour, minute, second;
					using formicine::util::parse_long;
					if (!parse_long(hour_str, hour) || !parse_long(minute_str, minute) ||
					    !parse_long(second_str, second))
						throw std::runtime_error("Invalid playback timestamp");
					window.queue([=, this] {
						window.box[channel].addMessage(name, content, is_self,   static_cast<int>(hour),
						                               static_cast<int>(minute), static_cast<int>(second));
					});
				} else {
					const auto &line = ev->line;
					window.queue([this, content, channel, name, is_self, line] {
						window.box[channel].addMessage(name, content, is_self, line.time.hours(), line.time.minutes(),
							line.time.seconds());
					});
				}
			} else if (ev->isUser()) {
				auto user = ev->speaker;
				auto speaker = user;
				const bool is_self = user->isSelf();
				if (is_self)
					user = ev->getUser(ev->server);
				window.queue([this, content, user, speaker, is_self] {
					window.box.add(user.get(), false);
					window.box[user].addMessage(speaker->name, content, is_self);
				});
			}
		});

		PingPong::Events::listen<PingPong::QuitEvent>([this](PingPong::QuitEvent *ev) {
			const auto &server = ev->server;
			if (ev->who->isSelf()) {
				window.queue([this, server] {
					window.box.erase(server);
				});
			} else {
				const auto &user = ev->who;
				const auto &reason = ev->content;
				window.queue([this, server, user, reason] {
					for (const std::weak_ptr<PingPong::Channel> &weak_channel: user->channels)
						if (auto channel = weak_channel.lock())
							if (window.box.hasLineView(channel.get()))
								window.box[channel.get()].quit(user->name, reason);
				});
			}
		});

		PingPong::Events::listen<PingPong::RawInEvent>([this](PingPong::RawInEvent *ev) {
			const auto &server = ev->server;
			auto raw = ev->rawIn;
			while (!raw.empty() && (raw.back() == '\r' || raw.back() == '\n'))
				raw.pop_back();
			window.queue([this, server, raw] {
				window.box.add(server, false);
				window.box[server].add("<< " + raw);
			});
		});

		PingPong::Events::listen<PingPong::RawOutEvent>([this](PingPong::RawOutEvent *ev) {
			const auto &server = ev->server;
			auto raw = ev->rawOut;
			while (!raw.empty() && (raw.back() == '\r' || raw.back() == '\n'))
				raw.pop_back();
			window.queue([this, server, raw] {
				window.box.add(server, false);
				window.box[server].add(">> " + raw);
			});
		});

		PingPong::Events::listen<PingPong::ServerStatusEvent>([this](PingPong::ServerStatusEvent *ev) {
			callInQueue(ev->server, ev->server->getStatus());

			switch (ev->server->getStatus()) {
				case PingPong::Server::Stage::Ready: {
					auto server = ev->server;
					window.queue([this, server] {
						window.box.add(server, true);
						window.box.addStatus("Connected to " + server->id + " (" + server->hostname + ":"
							+ std::to_string(server->port) + ")");
					});
					break;
				}
				case PingPong::Server::Stage::Dead: {
					auto server = ev->server;
					window.queue([this, server] {
						window.box.erase(server);
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

		PingPong::Events::listen<PingPong::TopicUpdatedEvent>([this](PingPong::TopicUpdatedEvent *ev) {
			auto channel = ev->channel;
			window.queue([this, channel] {
				window.box.setTopic(channel.get(), std::string(channel->topic));
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
		if (server->getStatus() == stage)
			fn();
		else
			serverStatusQueue[server].push_back({stage, fn});
	}
}
