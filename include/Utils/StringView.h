#pragma once

#if __has_include(<string_view>)
#include <string_view>
#else
#define STD_STRING_VIEW_IS_MISSING
#endif

#ifdef STD_STRING_VIEW_IS_MISSING
// TODO: Implement here!
#endif