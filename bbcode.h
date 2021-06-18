#ifndef BBCODE_H
#define BBCODE_H

#include <stdio.h>

typedef struct _bbcode_doc bbcode_doc;

bbcode_doc *bbcode_parse(size_t (*read)(void *, size_t, size_t, void *), void *data);

/* returns how much written or negative number if error */
int bbcode_print(
  bbcode_doc *doc,
  size_t (*write)(void *, size_t, size_t, void*),
  void *data,
  char *roorpath
);

void bbcode_doc_destroy(bbcode_doc *doc);

size_t bbcode_fwrite(void *ptr, size_t size, size_t nitems, void *stream);
size_t bbcode_fread(void *ptr, size_t size, size_t nitems, void *stream);

#endif
