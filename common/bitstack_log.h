#pragma once

#include <string>

#if defined(LLAMA_RAF_BITSTACK)
void bs_init(const char * path);
void bs_event(const char * tag, int code, const char * msg);
void bs_flush();
#else
inline void bs_init(const char *) {}
inline void bs_event(const char *, int, const char *) {}
inline void bs_flush() {}
#endif
