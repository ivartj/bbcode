#ifndef STRUT_H
#define STRUT_H

#include "bbcode.h"

bbcode_doc *makedoc(void);
bbcode *maketext(char *text, size_t len);
bbcode *makeb(void);
bbcode *makei(void);
bbcode *makeu(void);
bbcode *makes(void);
bbcode *makeurl(char *url, size_t urllen);
bbcode *makeimg(char *url, size_t urllen);
bbcode *makestop(int type);
void docadd(bbcode_doc *doc, bbcode *bb);

#endif
