#pragma once

#include <stdint.h>
#include <stddef.h>
#include <string>
#include <vector>
#include "XAC.hpp"
#include "parsers.h"

#define XAC_DEBUG 1

namespace ogl {
	char* write_xac(xac_context const& context, char* buffer);
}
