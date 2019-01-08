#ifndef LOG_H
#define LOG_H
#include "utf8.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
FILE *fp;

void initlog() {
  fp = fopen("/tmp/log.txt", "w+");
  fprintf(fp, "This is testing for fprintf...\n");
}
void vtermlog(const char *fmt, ...) {
  /* Log to file */
  if (fp) {
    va_list args;
    char buf[32];
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    fprintf(fp, "\n");
    fflush(fp);
  }
}
void getcell(VTermScreenCell *cell, char *buf) {
  unsigned char bytes[4];
  size_t count = codepoint_to_utf8(cell->chars[0], bytes);
  int k;
  for (k = 0; k < count; k++) {
    buf[k] = bytes[k];
  }
  buf[count] = '\0';
  return;
}
void vtermlogline(char *prefix, ScrollbackLine *line) {
  char buf[1024];
  char cellbuf[1024];
  strcpy(buf, prefix);
  int k;
  for (k = 0; k < line->cols; k++) {
    VTermScreenCell cell = line->cells[k];
    strcat(buf, "");
    getcell(&cell, cellbuf);
    strcat(buf, cellbuf);
  }
  strcat(buf, "end");
  vtermlog(buf);
}

void vtermlogbuffer(Term *term) {
  int k;
  for (k = 0; k < term->sb_current; k++) {
    ScrollbackLine *line = term->sb_buffer[k];
    char buf[10];
    sprintf(buf, "%d", k);

    vtermlogline(buf, line);
  }
  vtermlog("sb_current=%d", term->sb_current);
  vtermlog("sb_pending=%d", term->sb_pending);
  vtermlog("endofscrollback");
}

#endif
