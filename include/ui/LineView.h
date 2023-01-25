#pragma once
#include <iostream>
#include <gtkmm.h>
#include <ctime>
#include <deque>
#include <memory>
#include <variant>
#include <vector>

#include "pingpong/core/ModeSet.h"

namespace PingPong {
	class Channel;
	class Server;
	class User;
}

namespace Reden {
	class ConfigCache;

	class LineView: public Gtk::TextView {
		public:
			enum class ParentType {Other, Channel, Server, User};

			void *parent = nullptr;
			ParentType type = ParentType::Other;

			LineView() = delete;
			LineView(ConfigCache &);
			LineView(ConfigCache &cache_, PingPong::Channel *channel): LineView(cache_) {
				parent = channel;
				type = ParentType::Channel;
			}
			LineView(ConfigCache &cache_, PingPong::Server *server): LineView(cache_) {
				parent = server;
				type = ParentType::Server;
			}
			LineView(ConfigCache &cache_, PingPong::User *user): LineView(cache_) {
				parent = user;
				type = ParentType::User;
			}

			void loadProperties();
			LineView & add(const Glib::ustring &text, bool pangoize = true);
			LineView & addMessage(const Glib::ustring &name, const Glib::ustring &message, bool is_self = false);
			LineView & addMessage(const Glib::ustring &name, const Glib::ustring &message, bool is_self, int hour,
			                      int minute, int second);
			LineView & joined(const Glib::ustring &name, const Glib::ustring &channel);
			LineView & parted(const Glib::ustring &name, const Glib::ustring &channel, const Glib::ustring &reason);
			LineView & quit(const Glib::ustring &name, const Glib::ustring &reason);
			LineView & mode(std::shared_ptr<PingPong::Channel>, std::shared_ptr<PingPong::User>,
			                const PingPong::ModeSet &);
			LineView & topicChanged(std::shared_ptr<PingPong::Channel>, std::shared_ptr<PingPong::User>,
			                        const Glib::ustring &);
			LineView & topicIs(std::shared_ptr<PingPong::Channel>, const Glib::ustring &);
			LineView & nickChanged(const Glib::ustring &old_nick, const Glib::ustring &new_nick);
			/** For self nick changes. */
			LineView & nickChanged(const Glib::ustring &new_nick);
			LineView & error(const Glib::ustring &, bool is_markup = false);

			bool isAlive() const { return alive; }
			bool is(ParentType type_) const { return type == type_; }
			void clear();
			PingPong::Channel * getChannel() const;
			PingPong::Server  *  getServer() const;
			PingPong::User    *    getUser() const;
			bool isChannel() const { return type == ParentType::Channel; }
			bool  isServer() const { return type == ParentType::Server;  }
			bool    isUser() const { return type == ParentType::User;    }
			LineView & set(PingPong::Channel *);
			LineView & set(PingPong::Server *);
			LineView & set(PingPong::User *);

			static constexpr const char * nameColors[] {"red", "orange", "yellow", "green", "cyan", "purple", "pink"};
			static constexpr size_t colorCount = sizeof(nameColors) / sizeof(nameColors[0]);

		private:
			ConfigCache &cache;
			Glib::RefPtr<Gtk::TextTag> timeTag, bracketTag, nameTag, messageTag, plainTag, actionTag, channelTag,
			                           modesTag, userTag, topicTag, asteriskTag, selfTag, linkTag, name0, name1, name2,
			                           name3, name4, name5, name6, name7, name8, name9, name10, name11, name12, name13,
			                           name14, name15;
			bool alive = true;
			/** Whether the LineView was scrolled to the bottom at the beginning of the last call to start(). */
			bool wasAtEnd = false;
			Glib::RefPtr<Gtk::TextBuffer::Mark> endMark;
			std::vector<std::unique_ptr<Gtk::Widget>> widgets;
			Glib::RefPtr<Gtk::GestureClick> click;
			Glib::RefPtr<Gtk::EventControllerMotion> motion;

			static Glib::ustring makeTimestamp(int hour, int minute, int second);
			static Glib::ustring makeTimestamp(time_t);
			static Glib::ustring makeTimestamp();

			LineView & start();
			LineView & append(const Glib::ustring &, const Glib::ustring &tag_name);
			LineView & append(const Glib::ustring &);
			LineView & appendMarkup(const Glib::ustring &);
			LineView & addNewline();
			LineView & addTime();
			LineView & addTime(const Glib::ustring &timestamp);
			LineView & addStar();
			LineView & addMessageMain(const Glib::ustring &name, const Glib::ustring &message, bool is_self);
			void showURI(const Glib::ustring &);

			void setBold(Glib::RefPtr<Gtk::TextTag>);
			bool atEnd() const;
			LineView & scroll();
	};
}
