#pragma once

#ifdef _MSC_VER
# pragma warning(disable: 4251 4275)
#endif

#include "yasli/SharedLibApi.h"
#ifdef YASLI_EXPORTED
# define YASLI_API EXPORT_SYMBOL
#else
# define YASLI_API IMPORT_SYMBOL
#endif
