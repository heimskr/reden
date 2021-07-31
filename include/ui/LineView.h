#pragma once

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
	class LineView: public Gtk::TextView {
		public:
			enum class ParentType {Other, Channel, Server, User};

			void *parent = nullptr;
			ParentType type = ParentType::Other;

			LineView();
			LineView(PingPong::Channel *channel): parent(channel), type(ParentType::Channel) {}
			LineView(PingPong::Server *server): parent(server), type(ParentType::Server) {}
			LineView(PingPong::User *user): parent(user), type(ParentType::User) {}

			LineView & add(const Glib::ustring &text, bool pangoize = true);
			LineView & addMessage(const Glib::ustring &name, const Glib::ustring &message);
			LineView & joined(const Glib::ustring &name, const Glib::ustring &channel);
			LineView & mode(std::shared_ptr<PingPong::Channel>, std::shared_ptr<PingPong::User>,
			                const PingPong::ModeSet &);
			LineView & topicChanged(std::shared_ptr<PingPong::Channel>, std::shared_ptr<PingPong::User>,
			                        const Glib::ustring &);

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

		private:
			Glib::RefPtr<Gtk::TextTag> timeTag, bracketTag, nameTag, messageTag, plainTag, actionTag, channelTag,
			                           modesTag, userTag, topicTag, asteriskTag;
			bool alive = true;
			/** Whether the LineView was scrolled to the bottom at the beginning of the last call to start(). */
			bool wasAtEnd = false;
			Glib::RefPtr<Gtk::TextBuffer::Mark> endMark;

			static Glib::ustring makeTimestamp(time_t);
			static Glib::ustring makeTimestamp();

			LineView & start();
			LineView & append(const Glib::ustring &text, const Glib::ustring &tag_name);
			LineView & append(const Glib::ustring &text);
			LineView & addNewline();
			LineView & addTime();
			LineView & addStar();

			void setBold(Glib::RefPtr<Gtk::TextTag>);
			bool atEnd() const;
			LineView & scroll();
	};
}
