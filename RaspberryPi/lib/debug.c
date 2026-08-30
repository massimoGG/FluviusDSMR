#include "debug.h"

#include <stdint.h>
#include <stdio.h>

typedef struct {
  unsigned int foreground;
  unsigned int background;
} ansiiColours_t;

/* ANSII background colours have an offset of 10 */
const unsigned int c_backgroundColourOffset = 10; 

const unsigned int c_hexWidth = 16;

static ansiiColours_t getColoursFromLvl(debug_lvl_e lvl);
static void printColourPrefix(debug_lvl_e lvl);
static void printColourSuffix(void);

/**
 * @brief prints with a colour depending on the level
 * @note see https://en.wikipedia.org/wiki/ANSI_escape_code#3-bit_and_4-bit for colours
 * @details 
 * @param lvl level of message. See @e debug_lvl_e
 * @param tag tag of message, this can be __func__, __FILE__, etc...
 * @param fmt format of message, same as printf
 * @param ...
 */
void debug_log(debug_lvl_e lvl, const char *tag, const char *fmt, ...)
{
  printColourPrefix(lvl);

  printf("%s ", tag);

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  
  printColourSuffix();
}

void debug_log_hexdump(debug_lvl_e lvl, const volatile void *pBuf, unsigned short len)
{
  printColourPrefix(lvl);

  for (unsigned lineIdx = 0; lineIdx < len; lineIdx += c_hexWidth) {

    const uint8_t *pLineStart = (const uint8_t *)pBuf + lineIdx;

    unsigned lineLength = lineIdx + c_hexWidth < len ? c_hexWidth : len - lineIdx;

    /* Dump hex values */
    for (unsigned byteIndex = 0; byteIndex < lineLength; byteIndex++) {
      printf("%02x ", *(pLineStart + byteIndex));
    }

    printf("\t");
    
    /* Dump printable characters */
    for (unsigned byteIndex = 0; byteIndex < lineLength; byteIndex++) {
      const char ch = *(pLineStart + byteIndex);
      if (ch > 32 && ch < 126) {
        printf("%c",ch);
      } else {
        putchar('.');
      }
    }

    printf("\n");
  }

  printColourSuffix();
}


static ansiiColours_t getColoursFromLvl(debug_lvl_e lvl) {
  ansiiColours_t ret = {
    .foreground = eColour_white,
    .background = 0,  /* optional */
  };
  
  switch (lvl) {
  case eDebug_error:
    ret.foreground = eColour_white;
    ret.background = eColour_brightRed;
    break;
  case eDebug_warning:
    ret.foreground = eColour_white;
    ret.background = eColour_yellow;
    break;
  case eDebug_notice:
    ret.foreground = eColour_magenta;
    ret.background = eColour_white;
    break;
  case eDebug_info:
    ret.foreground = eColour_blue;
    break;
  case eDebug_debug:
    ret.foreground = eColour_green;
    break;
  case eDebug_verbose:
    ret.foreground = eColour_white;
    ret.background = eColour_blue;
    break;
  }

  ret.background += c_backgroundColourOffset;

  return ret;
}

static void printColourPrefix(debug_lvl_e lvl) {
  const ansiiColours_t colours = getColoursFromLvl(lvl);

  if (c_backgroundColourOffset != colours.background) {
    /* Apply background offset */
    printf("\x1b[%u;%um", colours.foreground, colours.background);
  } else {
    printf("\x1b[%um", colours.foreground);
  }
}

static void printColourSuffix(void)
{
  printf("\x1b[0m\n");
}