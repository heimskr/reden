#include "UI.h"
#include "core/Client.h"
#include "ui/BasicEntry.h"
#include "ui/ConnectDialog.h"
#include "ui/EntryDialog.h"
#include "ui/RedenWindow.h"

#include "lib/formicine/futil.h"

namespace Reden {
	ConnectDialog::ConnectDialog(const Glib::ustring &title, Gtk::Window &parent, Client &client_, bool modal):
	Gtk::Dialog(title, parent, modal), client(client_) {
		set_default_size(300, -1);
		auto &area = *get_content_area();
		area.set_orientation(Gtk::Orientation::VERTICAL);
		setMargins(area, 5);
		area.set_spacing(5);
		populateConfigs();
		comboModel = Gtk::ListStore::create(columns);
		serverCombo.set_model(comboModel);
		area.append(serverCombo);
		resetCombo();
		serverCombo.pack_start(columns.name);
		serverCombo.signal_changed().connect(sigc::mem_fun(*this, &ConnectDialog::on_combo_changed));
		hostEntry.set_placeholder_text("Hostname");
		portEntry.set_placeholder_text("Port");
		nickEntry.set_placeholder_text("Nickname");
		passwordEntry.set_placeholder_text("Password");
		passwordEntry.set_visibility(false);
		area.append(hostEntry);
		area.append(portEntry);
		area.append(nickEntry);
		area.append(passwordEntry);
		area.append(buttonBox);
		buttonBox.append(cancelButton);
		buttonBox.append(okButton);
		buttonBox.set_hexpand(true);
		buttonBox.set_halign(Gtk::Align::END);
		buttonBox.set_spacing(5);
		cancelButton.signal_clicked().connect(sigc::mem_fun(*this, &ConnectDialog::hide));
		okButton.signal_clicked().connect(sigc::mem_fun(*this, &ConnectDialog::submit));
		hostEntry.signal_activate().connect(sigc::mem_fun(*this, &ConnectDialog::submit));
		portEntry.signal_activate().connect(sigc::mem_fun(*this, &ConnectDialog::submit));
		nickEntry.signal_activate().connect(sigc::mem_fun(*this, &ConnectDialog::submit));
		passwordEntry.signal_activate().connect(sigc::mem_fun(*this, &ConnectDialog::submit));
	}

	void ConnectDialog::on_combo_changed() {
		if (const auto iter = serverCombo.get_active()) {
			const auto ptr = std::shared_ptr<ServerConfig>((*iter)[columns.config]);
			if (ptr) {
				fill(*ptr);
			} else {
				auto *sub = new EntryDialog<BasicEntry>("Save", *this, "Configuration name:", true);
				subdialog.reset(sub);
				sub->signal_submit().connect([this](const Glib::ustring &text) {
					if (text.empty())
						return;
					Glib::ustring error;
					long port;
					if (text.find_first_of("=\n\r\t\"") != Glib::ustring::npos)
						error = "Invalid name.";
					else if (!formicine::util::parse_long(portEntry.get_text(), port) || port < 1 || INT_MAX < port)
						error = "Invalid port.";

					if (!error.empty()) {
						client.window.queue([this, error] {
							auto *dialog = new Gtk::MessageDialog(*subdialog, error, false, Gtk::MessageType::ERROR,
								Gtk::ButtonsType::OK, true);
							subdialog.reset(dialog);
							dialog->signal_response().connect([this](int) {
								subdialog->close();
							});
							dialog->show();
						});
					} else {
						client.config.insert("servers", text, Glib::ustring(ServerConfig(hostEntry.get_text(),
							nickEntry.get_text(), passwordEntry.get_text(), static_cast<int>(port))), true);
						populateConfigs();
						resetCombo();
					}
				});
				subdialog->show();
			}
		}
	}

	void ConnectDialog::submit() {
		hide();
		signal_submit_.emit(hostEntry.get_text(), portEntry.get_text(), nickEntry.get_text(), passwordEntry.get_text());
	}

	void ConnectDialog::populateConfigs() {
		serverConfigs.clear();
		if (client.config.hasGroup("servers"))
			for (const auto &key: client.config.allKeys("servers"))
				serverConfigs.try_emplace(key, std::make_shared<ServerConfig>(client.config.getString("servers", key)));
	}

	void ConnectDialog::resetCombo() {
		comboModel->clear();
		if (!serverConfigs.empty()) {
			bool first = true;
			for (const auto &[name, config]: serverConfigs) {
				auto iter = comboModel->append();
				(*iter)[columns.name] = name;
				(*iter)[columns.config] = config;
				if (first) {
					serverCombo.set_active(iter);
					fill(*config);
					first = false;
				}
			}
		}
		auto iter = comboModel->append();
		(*iter)[columns.name] = "Save Configuration...";
		(*iter)[columns.config] = nullptr;
	}

	void ConnectDialog::fill(const ServerConfig &config) {
		hostEntry.set_text(config.host);
		portEntry.set_text(std::to_string(config.port));
		nickEntry.set_text(config.nick);
		passwordEntry.set_text(config.password);
	}

	ConnectDialog::ServerConfig::ServerConfig(const Glib::ustring &combined) {
		if (std::count(combined.begin(), combined.end(), ':') < 3)
			throw std::invalid_argument("Invalid combined string for ConnectDialog::ServerConfig");
		const size_t first  = combined.find(':'),
		             second = combined.find(':', first + 1),
		             third  = combined.find(':', second + 1);
		host = combined.substr(0, first);
		const Glib::ustring port_str = combined.substr(first + 1, second - first - 1);
		long long_port;
		if (!formicine::util::parse_long(port_str, long_port) || INT_MAX < long_port)
			throw std::invalid_argument("Invalid port.");
		port = static_cast<int>(long_port);
		nick = combined.substr(second + 1, third - second - 1);
		if (third != combined.size() - 1)
			password = combined.substr(third + 1);
	}

	ConnectDialog::ServerConfig::operator Glib::ustring() const {
		return host + ":" + std::to_string(port) + ":" + nick + ":" + password;
	}
}
