#pragma once

#include <gtkmm.h>
#include <map>
#include <memory>
#include <unordered_map>

#include "Connection.h"
#include "ui/BasicEntry.h"
#include "ui/LineView.h"
#include "ui/ServerTree.h"

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
			void addStatus(const std::string &, bool pangoize = true);
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
			void log(const Glib::ustring &, bool pangoize = false);

			bool hasLineView(void *ptr) const;
			LineView & getLineView(void *ptr);
			LineView & getLineView(PingPong::Channel *);
			LineView & getLineView(PingPong::Server *);
			LineView & getLineView(PingPong::User *);
			const LineView & getLineView(void *ptr) const;
			LineView & operator[](void *ptr);
			const LineView & operator[](void *ptr) const;
			void setTopic(void *ptr, const std::string &);
			void add(PingPong::Channel *, bool focus = false);
			void add(PingPong::Server *, bool focus = false);
			void add(PingPong::User *, bool focus = false);
			void erase(PingPong::Channel *);
			void erase(PingPong::Server *);
			void erase(PingPong::User *);

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
			ServerTree serverTree;
			Gtk::TreeView userTree;
			Glib::RefPtr<Gtk::ListStore> userModel;
			Gtk::Separator leftSeparator, rightSeparator, topicSeparator;
			Gtk::Box chatBox {Gtk::Orientation::VERTICAL};
			Gtk::Label topicLabel {"", Gtk::Align::START};
			Gtk::ScrolledWindow serversScrolled, usersScrolled, chatScrolled, topicScrolled;
			BasicEntry chatEntry;
			Columns columns;
			Glib::RefPtr<Gtk::EventControllerKey> keyController;
			std::unordered_map<void *, LineView> views;
			std::unordered_map<void *, Glib::ustring> topics;
			std::unordered_map<std::string, Gtk::TreeModel::iterator> presentUserRows;
			std::unordered_map<void *, std::set<std::string>> userSets;

			Client & client();

			void addStatusRow();
			void focusView(void *);
			void focusView(void *, LineView * &);
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
