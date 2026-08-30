#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>

typedef enum {
  eColour_black = 30,
  eColour_red,
  eColour_green,
  eColour_yellow,
  eColour_blue,
  eColour_magenta,
  eColour_cyan,
  eColour_white,
  eColour_gray = 90,
  eColour_brightRed,
  eColour_brightGreen,
  eColour_brightYellow,
  eColour_brightBlue,
  eColour_brightMagenta,
  eColour_brightCyan,
  eColour_brightWhite,
} ansiColour_e;

typedef enum {
  eDebug_error,
  eDebug_warning,
  eDebug_notice,
  eDebug_info,
  eDebug_debug,
  eDebug_verbose,
} debug_lvl_e;

void debug_log(debug_lvl_e lvl, const char *tag, const char *fmt, ...);
void debug_log_hexdump(debug_lvl_e lvl, const volatile void *pBuf, unsigned short len);

#define DBG_ERR(...) debug_log(eDebug_error, __func__, __VA_ARGS__)
#define DBG_WARN(...) debug_log(eDebug_warning, __func__, __VA_ARGS__)
#define DBG_NOTICE(...) debug_log(eDebug_notice, __func__, __VA_ARGS__)
#define DBG_INFO(...) debug_log(eDebug_info, __func__, __VA_ARGS__)
#define DBG_DBG(...) debug_log(eDebug_debug, __func__, __VA_ARGS__)
#define DBG_VERBOSE(...) debug_log(eDebug_verbose, __func__, __VA_ARGS__)

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif