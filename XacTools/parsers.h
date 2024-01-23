#pragma once

#include <string>

namespace parsers {
	struct error_handler {
		std::string accumulated_errors;
		std::string accumulated_warnings;
		std::string file_name;
	};
}
