#include "UI.h"
#include "ui/ConnectDialog.h"

namespace Reden {
	ConnectDialog::ConnectDialog(const Glib::ustring &title, Gtk::Window &parent, bool modal):
	Gtk::Dialog(title, parent, modal) {
		set_default_size(300, -1);
		auto &area = *get_content_area();
		area.set_orientation(Gtk::Orientation::VERTICAL);
		setMargins(area, 5);
		area.set_spacing(5);
		hostEntry.set_placeholder_text("Hostname");
		portEntry.set_placeholder_text("Port");
		nickEntry.set_placeholder_text("Nickname");
		portEntry.set_text("6667");
		area.append(hostEntry);
		area.append(portEntry);
		area.append(nickEntry);
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
	}

	void ConnectDialog::submit() {
		hide();
		signal_submit_.emit(hostEntry.get_text(), portEntry.get_text(), nickEntry.get_text());
	}
}
