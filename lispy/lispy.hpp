#pragma once

#include "types.hpp"

namespace lispy {

LISPY_EXPORT Program*
compile(std::string::iterator& begin, std::string::iterator& end);

LISPY_EXPORT HashMap*
setup_stdctx();

}
