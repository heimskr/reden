#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>
#include <unordered_map>

#include "Connection.h"
#include "ui/BasicEntry.h"
#include "ui/LineView.h"

namespace PingPong {
	class Server;
	class Channel;
	class User;
}

namespace Reden {
	class RedenWindow;
	class Client;

	class MainBox: public Gtk::Box {
		public:
			MainBox() = delete;
			MainBox(RedenWindow &);
			void *activeView = nullptr;

			void focusEntry();
			void addServer(PingPong::Server *, bool focus = false);
			void addChannel(PingPong::Channel *, bool focus = false);
			void eraseServer(PingPong::Server *);
			void eraseChannel(PingPong::Channel *);
			void addStatus(const std::string &);
			void updateChannel(PingPong::Channel &);
			Glib::ustring getInput() const;
			void setInput(const Glib::ustring &);
			int getCursor() const;
			void setCursor(int);
			bool inStatus() const;
			LineView & active();
			PingPong::Server * activeServer();
			PingPong::Channel * activeChannel();
			PingPong::User * activeUser();
			void log(const Glib::ustring &);

			LineView & getLineView(void *ptr);
			LineView & getLineView(PingPong::Channel *);
			LineView & getLineView(PingPong::Server *);
			LineView & getLineView(PingPong::User *);
			const LineView & getLineView(void *ptr) const;
			LineView & operator[](void *ptr);
			const LineView & operator[](void *ptr) const;
			void setTopic(void *ptr, const std::string &);

			template <typename T>
			LineView & getLineView(std::shared_ptr<T> ptr) {
				return getLineView(ptr.get());
			}

			template <typename T>
			const LineView & getLineView(std::shared_ptr<T> ptr) const {
				return getLineView(ptr.get());
			}

			template <typename T>
			LineView & operator[](std::shared_ptr<T> ptr) {
				return (*this)[ptr.get()];
			}

			template <typename T>
			const LineView & operator[](std::shared_ptr<T> ptr) const {
				return (*this)[ptr.get()];
			}

		private:
			struct Columns: public Gtk::TreeModelColumnRecord {
				Columns() {
					add(name);
					add(pointer);
				}

				Gtk::TreeModelColumn<Glib::ustring> name;
				Gtk::TreeModelColumn<void *> pointer;
			};

			RedenWindow &parent;
			Gtk::TreeView serverTree, userTree;
			Glib::RefPtr<Gtk::TreeStore> serverModel;
			Glib::RefPtr<Gtk::ListStore> userModel;
			Gtk::Separator leftSeparator, rightSeparator, topicSeparator;
			Gtk::Box chatBox {Gtk::Orientation::VERTICAL};
			Gtk::Label topicLabel {"", Gtk::Align::START};
			Gtk::ScrolledWindow serversScrolled, usersScrolled, chatScrolled, topicScrolled;
			Gtk::Grid chatGrid;
			BasicEntry chatEntry;
			Columns columns;
			Glib::RefPtr<Gtk::EventControllerKey> keyController;
			// I'm using a void pointer here because serverRows can store both PingPong::Server pointers and also a
			// dummy pointer to this MainBox for the status window.
			std::unordered_map<void *, Gtk::TreeModel::iterator> serverRows;
			std::unordered_map<PingPong::Channel *, Gtk::TreeModel::iterator> channelRows;
			std::unordered_map<void *, LineView> views;
			std::unordered_map<void *, Glib::ustring> topics;
			std::unordered_map<std::string, Gtk::TreeModel::iterator> userRows;
			std::unordered_map<void *, std::set<std::string>> userSets;

			Client & client();

			void addStatusRow();
			void focusView(void *);
			void focusView(void *, LineView * &);
			void serverRowActivated(const Gtk::TreeModel::Path &, Gtk::TreeViewColumn *);
			int compareUsers(const Gtk::TreeModel::const_iterator &, const Gtk::TreeModel::const_iterator &);
			bool keyPressed(guint, guint, Gdk::ModifierType);
			void entryActivated();

			template <typename T>
			void focusView(void *ptr, T *parent) {
				LineView *view;
				focusView(ptr, view);
				if (!view->parent)
					view->set(parent);
			}
	};
}
