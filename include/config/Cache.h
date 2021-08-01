#pragma once

#include "config/Defaults.h"

namespace Reden {
	class ConfigCache {
		private:
			static const Glib::ustring & getString(const Glib::ustring &key);
			static bool getBool(const Glib::ustring &key);
			static long getLong(const Glib::ustring &key);

		public:
			bool appearanceAllowEmptyHats = getBool("appearance.allow_empty_hats");
			
			bool behaviorAnswerVersionRequests = getBool("behavior.answer_version_requests"),
			     behaviorHideVersionRequests   = getBool("behavior.hide_version_requests");
			
			Glib::ustring completionPingSuffix = getString("completion.ping_suffix");

			bool debugShowRaw = getBool("debug.show_raw");

			bool interfaceCloseOnPart  = getBool("interface.close_on_part"),
			     interfaceShowMotds    = getBool("interface.show_motds");

			Glib::ustring interfacePlaybackMode = getString("interface.playback_mode");

			bool messagesDirectOnly       = getBool("messages.direct_only"),
			     messagesHighlightNotices = getBool("messages.highlight_notices"),
			     messagesNoticesInStatus  = getBool("messages.notices_in_status");

			Glib::ustring serverDefaultNick = getString("server.default_nick"),
			              serverDefaultUser = getString("server.default_user"),
			              serverDefaultReal = getString("server.default_real");

			Glib::ustring noticeForeground = getString("appearance.notice_foreground");

#define DEF_FORMAT(n, u) Glib::ustring format##n = getString("format."#u);
			DEF_FORMAT(Action, action);
			DEF_FORMAT(Bang, bang);
			DEF_FORMAT(BangBad, bang_bad);
			DEF_FORMAT(BangGood, bang_good);
			DEF_FORMAT(BangWarn, bang_warn);
			DEF_FORMAT(Channel, channel);
			DEF_FORMAT(HeaderAction, header_action);
			DEF_FORMAT(HeaderPrivmsg, header_privmsg);
			DEF_FORMAT(HeaderNotice, header_notice);
			DEF_FORMAT(Join, join);
			DEF_FORMAT(Kick, kick);
			DEF_FORMAT(KickSelf, kick_self);
			DEF_FORMAT(MessageAction, message_action);
			DEF_FORMAT(MessagePrivmsg, message_privmsg);
			DEF_FORMAT(MessageNotice, message_notice);
			DEF_FORMAT(NickAction, nick_action);
			DEF_FORMAT(NickChange, nick_change);
			DEF_FORMAT(NickGeneral, nick_general);
			DEF_FORMAT(NickGeneralBright, nick_general_bright);
			DEF_FORMAT(NickPrivmsg, nick_privmsg);
			DEF_FORMAT(NickNotice, nick_notice);
			DEF_FORMAT(Notice, notice);
			DEF_FORMAT(Part, part);
			DEF_FORMAT(Privmsg, privmsg);
			DEF_FORMAT(Quit, quit);
			DEF_FORMAT(Reason, reason);
			DEF_FORMAT(SelfNickChange, self_nick_change);
			DEF_FORMAT(Timestamp, timestamp);
			DEF_FORMAT(Topic, topic);
			DEF_FORMAT(TopicIs, topic_is);
			DEF_FORMAT(TopicChange, topic_change);
#undef DEF_FORMAT
	};
}
