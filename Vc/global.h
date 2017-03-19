// In older versions of Vc, version.h depended on global.h for the namespace
// macros, so libraries like HPX would include <Vc/global.h> to determine whether
// to enable Vc version 1 or Vc version 2 code. version.h has been fixed to no
// longer depend on global.h, but people still need to be able to include
// <Vc/global.h> so older versions of Vc will still work with our codebase. 

#include "detail/global.h"

// vim: ft=cpp
