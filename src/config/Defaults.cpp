#include <algorithm>
#include <cstdint>

#include "pingpong/core/IRC.h"

#include "core/Client.h"
#include "config/ConfigDB.h"
#include "config/Defaults.h"

#include "lib/formicine/ansi.h"

namespace Reden {
	RegisteredMap registered;
	std::map<Glib::ustring, Validator> validators;

	ValidationResult validateLong(const Value &val) {
		return std::holds_alternative<long>(val)? ValidationResult::Valid : ValidationResult::BadType;
	}

	ValidationResult validateNonnegative(const Value &val) {
		return std::holds_alternative<long>(val) && 0 <= std::get<long>(val)?
			ValidationResult::Valid : ValidationResult::BadType;
	}

	ValidationResult validateUint32(const Value &val) {
		return std::holds_alternative<long>(val) && 0 <= std::get<long>(val) && std::get<long>(val) <= UINT32_MAX?
			ValidationResult::Valid : ValidationResult::BadType;
	}

	ValidationResult validateInt32nn(const Value &val) {
		return std::holds_alternative<long>(val) && 0 <= std::get<long>(val) && std::get<long>(val) <= INT32_MAX?
			ValidationResult::Valid : ValidationResult::BadType;
	}

	ValidationResult validateString(const Value &val) {
		return std::holds_alternative<Glib::ustring>(val)? ValidationResult::Valid : ValidationResult::BadType;
	}

	ValidationResult validateBool(const Value &val) {
		return std::holds_alternative<bool>(val)? ValidationResult::Valid : ValidationResult::BadType;
	}

	static ValidationResult validatePlayback(const Value &value) {
		if (!std::holds_alternative<Glib::ustring>(value))
			return ValidationResult::BadType;
		const Glib::ustring &str = std::get<Glib::ustring>(value);
		if (str == "ignore" || str == "integrate" || str == "default")
			return ValidationResult::Valid;
		return ValidationResult::BadValue;
	}

	bool registerKey(const Glib::ustring &group, const Glib::ustring &key, const Value &default_value,
	                 const Validator &validator, const Applicator &applicator, const Glib::ustring &description) {
		Glib::ustring combined = group + "." + key;

		if (registered.count(group + "." + key) > 0)
			return false;

		registered.insert({combined, DefaultConfigKey(combined, default_value, validator, applicator, description)});
		return true;
	}

	bool unregister(const Glib::ustring &group, const Glib::ustring &key) {
		Glib::ustring combined = group + "." + key;
		if (registered.count(group + "." + key) == 0)
			return false;
		registered.erase(combined);
		return true;
	}

	void applyDefaults(ConfigDB &db) {
		for (auto &pair: registered)
			pair.second.apply(db, pair.second.defaultValue);
	}

	std::vector<Glib::ustring> startsWith(const Glib::ustring &str) {
		std::vector<Glib::ustring> out;
		for (const std::pair<const Glib::ustring, DefaultConfigKey> &pair: registered) {
			const Glib::ustring &full = pair.first;
			const Glib::ustring &key = full.substr(full.find('.') + 1);
			if (key.find(str) == 0 || full.find(str) == 0)
				out.push_back(full);
		}

		return out;
	}

	void registerDefaults() {
		// Appearance

		registerAppearance();

		// Behavior

		registerKey("behavior", "answer_version_requests", true, validateBool,
			CACHE_BOOL(behaviorAnswerVersionRequests), "Whether to respond to CTCP VERSION requests.");

		registerKey("behavior", "hide_version_requests", true, validateBool,
			CACHE_BOOL(behaviorHideVersionRequests), "Whether to handle CTCP VERSION requests silently.");

		// Completion

		registerKey("completion", "ping_suffix", ":", validateString, CACHE_STRING(completionPingSuffix),
			"The suffix to put after a user's name after tab completing their name in the first word of the message.");

		// Debug

		registerKey("debug", "show_raw", false, validateBool, CACHE_BOOL(debugShowRaw),
			"Whether to log raw input/output in the status window.");

		// Format

		registerFormat();

		// Interface

		registerKey("interface", "close_on_part", true, validateBool, CACHE_BOOL(interfaceCloseOnPart),
			"Whether to close a channel's window after parting it.");

		registerKey("interface", "show_motds", true, validateBool, CACHE_BOOL(interfaceShowMotds),
			"Whether to show server MOTDs.");

		registerKey("interface", "playback_mode", "integrate", validatePlayback, CACHE_STRING(interfacePlaybackMode),
			"How to handle ZNC playback. Valid values: ignore, integrate, default.");

		// Messages

		registerKey("messages", "direct_only", false, validateBool, CACHE_BOOL(messagesDirectOnly),
			"Whether to count only messages that begin with your name as highlights.");

		registerKey("messages", "highlight_notices", true, validateBool, CACHE_BOOL(messagesHighlightNotices),
			"Whether to treat all notices as highlights.");

		registerKey("messages", "notices_in_status", true, validateBool, CACHE_BOOL(messagesNoticesInStatus),
			"Whether non-channel notices should be displayed in the status window instead of a user window.");

		// Server

		registerKey("server", "default_nick", PingPong::IRC::defaultNick, validateString,
			CACHE_STRING(serverDefaultNick), "The nickname to use when connecting to a new server.");

		registerKey("server", "default_real", PingPong::IRC::defaultRealname, validateString,
		             [](ConfigDB &db, const Value &new_val) {
			db.parent.cache.serverDefaultReal = db.parent.irc()->realname = std::get<Glib::ustring>(new_val);
		}, "The real name to use when connecting to a new server.");

		registerKey("server", "default_user", PingPong::IRC::defaultUser, validateString,
		             [](ConfigDB &db, const Value &new_val) {
			db.parent.cache.serverDefaultUser = db.parent.irc()->username = std::get<Glib::ustring>(new_val);
		}, "The username to use when connecting to a new server.");
	}

	void registerAppearance() {
		// Notices
		registerKey("appearance", "notice_foreground", "magenta", validateString, CACHE_STRING(noticeForeground),
			"The text color of names in notice messages.");

		// Miscellaneous
		registerKey("appearance", "allow_empty_hats", false, validateBool, CACHE_BOOL(appearanceAllowEmptyHats),
			"Whether to use a blank hat string instead of a space when a user has no hats.");
	}

	void registerFormat() {
		registerKey("format", "action", "$header$ $message$", validateString, CACHE_STRING(formatAction),
			"The format string for actions. Available variables: header, message.");

		registerKey("format", "bang", "^d-^b!^D^d-^D", validateString, CACHE_STRING(formatBang),
			"The format string for neutral bangs. Bangs are the little pieces of text in a line that indicate the "
			"urgency of the line.");

		registerKey("format", "bang_bad", "^[red]^d-^D!^D^d-^D^[/f]", validateString, CACHE_STRING(formatBangBad),
			"The format string for bangs that indicate an error.");

		registerKey("format", "bang_good", "^[green]^d-^D!^D^d-^D^[/f]", validateString, CACHE_STRING(formatBangGood),
			"The format string for bangs that indicate success.");

		registerKey("format", "bang_warn", "^[yellow]^d-^D!^D^d-^D^[/f]", validateString, CACHE_STRING(formatBangWarn),
			"The format string for bangs that indicate a warning.");

		registerKey("format", "channel", "^b$raw_channel$^B", validateString, CACHE_STRING(formatChannel),
			"The format string for channels in messages like joins. Available variables: raw_channel.");

		registerKey("format", "header_action", "^b* $hats$$nick$^0", validateString,
			CACHE_STRING(formatHeaderAction),
			"The format string for headers in actions. Available variables: hats, nick.");

		registerKey("format", "header_privmsg", "^d<^D$hats$$nick$^d>^D", validateString,
			CACHE_STRING(formatHeaderPrivmsg),
			"The format string for headers in privmsgs. Available variables: hats, nick.");

		registerKey("format", "header_notice", "^[magenta]-$hats$$nick$^0^[magenta]-^0", validateString,
			CACHE_STRING(formatHeaderNotice),
			"The format string for headers in notices. Available variables: hats, nick.");

		registerKey("format", "join", "$!$ $who$^0 joined $channel$^0",
			validateString, CACHE_STRING(formatJoin),
			"The format string for joins. Available variables: -!-, -!!-, -!?-, channel, who.");

		registerKey("format", "kick", "$!$ $whom$^0 was kicked from $channel$^0 by $who$^0 ^d[^D$reason$^0^d]^0",
			validateString, CACHE_STRING(formatKick),
			"The format string for kicks. Available variables: -!-, -!!-, -!?-, channel, reason, who, whom.");

		registerKey("format", "kick_self", "$bad!$ You were kicked from $channel$^0 by $who$^0 ^d[^D$reason$^0^d]"
			"^0", validateString, CACHE_STRING(formatKickSelf),
			"The format string for kicks when you're the kicked user. Available variables: -!-, -!!-, -!?-, channel, "
			"reason, who, whom.");

		registerKey("format", "message_action", "$raw_message$", validateString,
			CACHE_STRING(formatMessageAction),
			"The format string for messages in actions. Available variables: hats, nick.");

		registerKey("format", "message_privmsg", "$raw_message$", validateString,
			CACHE_STRING(formatMessagePrivmsg),
			"The format string for messages in privmsgs. Available variables: hats, nick.");

		registerKey("format", "message_notice", "$raw_message$", validateString,
			CACHE_STRING(formatMessageNotice),
			"The format string for messages in notices. Available variables: hats, nick.");

		registerKey("format", "notice", "$header$ $message$", validateString, CACHE_STRING(formatNotice),
			"The format string for notices. Available variables: header, notice.");

		registerKey("format", "nick_action", "$raw_nick$", validateString, CACHE_STRING(formatNickAction),
			"The format string for nicks in actions. Available variables: raw_nick.");

		registerKey("format", "nick_change", "$old$ is now known as $new$", validateString,
			CACHE_STRING(formatNickChange), "The format string for nick changes. Available variables: new, old.");

		registerKey("format", "nick_general_bright", "^[cyan!]$raw_nick$^[/f]", validateString,
			CACHE_STRING(formatNickGeneralBright),
			"The format string for nicks in messages like joins. Available variables: raw_nick.");

		registerKey("format", "nick_general", "^[cyan]$raw_nick$^[/f]", validateString,
			CACHE_STRING(formatNickGeneral),
			"The format string for nicks in messages like quits and parts. Available variables: raw_nick.");

		registerKey("format", "nick_privmsg", "$raw_nick$", validateString, CACHE_STRING(formatNickPrivmsg),
			"The format string for nicks in privmsgs. Available variables: raw_nick.");

		registerKey("format", "nick_notice", "$raw_nick$", validateString, CACHE_STRING(formatNickNotice),
			"The format string for nicks in notices. Available variables: raw_nick.");

		registerKey("format", "part", "$!$ $who$^0 left $channel$^0 ^d[^D$reason$^0^d]^0", validateString,
			CACHE_STRING(formatPart),
			"The format string for parts. Available variables: -!-, -!!-, -!?-, channel, reason, who.");

		registerKey("format", "privmsg", "$header$ $message$", validateString, CACHE_STRING(formatPrivmsg),
			"The format string for privmsgs. Available variables: header, message.");

		registerKey("format", "reason", "$raw_reason$", validateString, CACHE_STRING(formatReason),
			"The format string for reasons. Available variables: raw_reason.");

		registerKey("format", "quit", "$!$ $who$^0 quit ^d[^D$reason$^0^d]^0",
			validateString, CACHE_STRING(formatQuit),
			"The format string for quits. Available variables: -!-, -!!-, -!?-, reason, who.");

		registerKey("format", "self_nick_change", "You are now known as $new$", validateString,
			CACHE_STRING(formatSelfNickChange), "The format string for self nick changes. Available variables: new.");

		registerKey("format", "timestamp", "%H^d:^D%M^d:^D%S", validateString, CACHE_STRING(formatTimestamp),
			"The format string for timestamps. Uses strftime syntax. Shouldn't exceed 32 characters when rendered.");

		registerKey("format", "topic", "$raw_topic$", validateString, CACHE_STRING(formatTopic),
			"The format string for topics. Available variables: raw_topic.");

		registerKey("format", "topic_is", "$!$ Topic for $channel$ is $topic$", validateString,
			CACHE_STRING(formatTopicIs), "The format string for topic notices. Available variables: -!-, -!!-, -!?-, "
			"channel, topic.");

		registerKey("format", "topic_change", "$!$ $who$ changed the topic of $channel$ to $topic$", validateString,
			CACHE_STRING(formatTopicChange), "The format string for topic changes. Available variables: -!-, -!!-, "
			"-!?-, channel, topic, who.");
	}
}
