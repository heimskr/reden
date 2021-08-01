#pragma once

#include <gtkmm.h>
#include <stdexcept>
#include <string>

namespace Reden {
	enum class ValidationResult {Valid, BadType, BadValue};

	class ValidationFailure: public std::exception {
		public:
			ValidationResult result;

			ValidationFailure(ValidationResult result_): result(result_) {
				switch (result) {
					case ValidationResult::Valid:
						error = "Configuration validated successfully but threw anyway, somehow?";
						break;
					case ValidationResult::BadType:
						error = "Invalid value type";
						break;
					case ValidationResult::BadValue:
						error = "Invalid value";
						break;
					default:
						error = "Unknown ValidationResult???";
				}
			}

			ValidationFailure(ValidationResult result_, const Glib::ustring &group, const Glib::ustring &key):
			result(result_) {
				switch (result) {
					case ValidationResult::Valid:
						error = "Configuration validated successfully but threw anyway, somehow for " + group + "."
						        + key + "?";
						break;
					case ValidationResult::BadType:
						error = "Invalid value type for " + group + "." + key;
						break;
					case ValidationResult::BadValue:
						error = "Invalid value for " + group + "." + key;
						break;
					default:
						error = "Unknown ValidationResult for " + group + "." + key + "???";
				}
			}

			const char * what() const noexcept {
				return error.c_str();
			}

		private:
			std::string error;
	};
}
