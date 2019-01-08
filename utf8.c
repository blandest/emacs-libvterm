#include "utf8.h"

size_t codepoint_to_utf8(const uint32_t codepoint, unsigned char buffer[4]) {
  if (codepoint <= 0x7F) {
    buffer[0] = codepoint;
    return 1;
  }
  if (codepoint >= 0x80 && codepoint <= 0x07FF) {
    buffer[0] = 0xC0 | (codepoint >> 6);
    buffer[1] = 0x80 | (codepoint & 0x3F);
    return 2;
  }
  if (codepoint >= 0x0800 && codepoint <= 0xFFFF) {
    buffer[0] = 0xE0 | (codepoint >> 12);
    buffer[1] = 0x80 | ((codepoint >> 6) & 0x3F);
    buffer[2] = 0x80 | (codepoint & 0x3F);
    return 3;
  }

  if (codepoint >= 0x10000 && codepoint <= 0x10FFFF) {
    buffer[0] = 0xF0 | (codepoint >> 18);
    buffer[1] = 0x80 | ((codepoint >> 12) & 0x3F);
    buffer[2] = 0x80 | ((codepoint >> 6) & 0x3F);
    buffer[3] = 0x80 | (codepoint & 0x3F);
    return 4;
  }
  return 0;
}

bool utf8_to_codepoint(const unsigned char buffer[4], const size_t len,
                       uint32_t *codepoint) {
  *codepoint = 0;
  if (len == 1 && buffer[0] <= 0x7F) {
    *codepoint = buffer[0];
    return true;
  }
  if (len == 2 && (buffer[0] >= 0xC0 && buffer[0] <= 0xDF) &&
      (buffer[1] >= 0x80 && buffer[1] <= 0xBF)) {
    *codepoint = buffer[0] & 0x1F;
    *codepoint = *codepoint << 6;
    *codepoint = *codepoint | (buffer[1] & 0x3F);
    return true;
  }
  if (len == 3 && (buffer[0] >= 0xE0 && buffer[0] <= 0xEF) &&
      (buffer[1] >= 0x80 && buffer[1] <= 0xBF) &&
      (buffer[2] >= 0x80 && buffer[2] <= 0xBF)) {
    *codepoint = buffer[0] & 0xF;
    *codepoint = *codepoint << 6;
    *codepoint = *codepoint | (buffer[1] & 0x3F);
    *codepoint = *codepoint << 6;
    *codepoint = *codepoint | (buffer[2] & 0x3F);
    return true;
  }
  if (len == 4 && (buffer[0] >= 0xF0 && buffer[0] <= 0xF7) &&
      (buffer[1] >= 0x80 && buffer[1] <= 0xBF) &&
      (buffer[2] >= 0x80 && buffer[2] <= 0xBF) &&
      (buffer[3] >= 0x80 && buffer[3] <= 0xBF)) {
    *codepoint = buffer[0] & 7;
    *codepoint = *codepoint << 6;
    *codepoint = *codepoint | (buffer[1] & 0x3F);
    *codepoint = *codepoint << 6;
    *codepoint = *codepoint | (buffer[2] & 0x3F);
    *codepoint = *codepoint << 6;
    *codepoint = *codepoint | (buffer[3] & 0x3F);
    return true;
  }

  return false;
}

/* almost copy from  https://github.com/GNOME/glib/blob/master/glib/gutf8.c */
#define UTF8_COMPUTE(Char, Mask, Len)                                          \
  if (Char < 128) {                                                            \
    Len = 1;                                                                   \
    Mask = 0x7f;                                                               \
  } else if ((Char & 0xe0) == 0xc0) {                                          \
    Len = 2;                                                                   \
    Mask = 0x1f;                                                               \
  } else if ((Char & 0xf0) == 0xe0) {                                          \
    Len = 3;                                                                   \
    Mask = 0x0f;                                                               \
  } else if ((Char & 0xf8) == 0xf0) {                                          \
    Len = 4;                                                                   \
    Mask = 0x07;                                                               \
  } else if ((Char & 0xfc) == 0xf8) {                                          \
    Len = 5;                                                                   \
    Mask = 0x03;                                                               \
  } else if ((Char & 0xfe) == 0xfc) {                                          \
    Len = 6;                                                                   \
    Mask = 0x01;                                                               \
  } else                                                                       \
    Len = -1;

#define UTF8_GET(Result, Chars, Count, Mask, Len)                              \
  (Result) = (Chars)[0] & (Mask);                                              \
  for ((Count) = 1; (Count) < (Len); ++(Count)) {                              \
    if (((Chars)[(Count)] & 0xc0) != 0x80) {                                   \
      (Result) = -1;                                                           \
      break;                                                                   \
    }                                                                          \
    (Result) <<= 6;                                                            \
    (Result) |= ((Chars)[(Count)] & 0x3f);                                     \
  }

/**
 * almost copy from  https://github.com/GNOME/glib/blob/master/glib/gutf8.c#L314
 * utf8_get_char:
 * @p: a pointer to Unicode character encoded as UTF-8
 *
 * Converts a sequence of bytes encoded as UTF-8 to a Unicode character.
 *
 * If @p does not point to a valid UTF-8 encoded character, results
 * are undefined. If you are not sure that the bytes are complete
 * valid Unicode characters, you should use utf8_get_char_validated()
 * instead.
 *
 * Returns: the resulting character
 */
uint32_t utf8_get_char(const unsigned char *p) {
  int i, mask = 0, len;
  uint32_t result;
  unsigned char c = (unsigned char)*p;

  UTF8_COMPUTE(c, mask, len);
  if (len == -1)
    return (uint32_t)-1;
  UTF8_GET(result, p, i, mask, len);

  return result;
}
