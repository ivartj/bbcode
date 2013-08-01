#include "strut.h"
#include "string.h"
#include <stdlib.h>

static char *textcopy(char *text, size_t len);

bbcode_doc *makedoc(void)
{
	bbcode_doc *doc;

	doc = calloc(1, sizeof(bbcode_doc));
	doc->type = BBCODE_DOC;

	return doc;
}

bbcode *maketext(char *text, size_t len)
{
	bbcode_text *bbtext;

	bbtext = calloc(1, sizeof(bbcode_text));
	bbtext->type = BBCODE_TEXT;
	bbtext->text = textcopy(text, len);
	bbtext->len = len;

	return (bbcode *)bbtext;
}

bbcode *makeb(void)
{
	bbcode_b *b;

	b = calloc(1, sizeof(bbcode_b));
	b->type = BBCODE_B;

	return (bbcode *)b;
}

bbcode *makei(void)
{
	bbcode_i *i;

	i = calloc(1, sizeof(bbcode_i));
	i->type = BBCODE_I;

	return (bbcode *)i;
}

bbcode *makeu(void)
{
	bbcode_u *u;

	u = calloc(1, sizeof(bbcode_u));
	u->type = BBCODE_U;

	return (bbcode *)u;
}

bbcode *makes(void)
{
	bbcode_s *s;

	s = calloc(1, sizeof(bbcode_i));
	s->type = BBCODE_S;

	return (bbcode *)s;
}

bbcode *makeurl(char *urlstr, size_t urllen)
{
	bbcode_url *url;

	url = calloc(1, sizeof(bbcode_url));
	url->type = BBCODE_URL;
	url->url = textcopy(urlstr, urllen);
	url->len = urllen;

	return (bbcode *)url;
}

bbcode *makeimg(char *url, size_t urllen)
{
	bbcode_img *img;

	img = calloc(1, sizeof(bbcode_img));
	img->type = BBCODE_IMG;
	img->url = textcopy(url, urllen);
	img->urllen = urllen;

	return (bbcode *)img;
}

bbcode *makestop(int type)
{
	bbcode_stop *stop;

	stop = calloc(1, sizeof(bbcode_stop));
	stop->type = BBCODE_STOP;
	stop->stoptype = type;

	return (bbcode *)stop;
}

void docadd(bbcode_doc *doc, bbcode *bb)
{
	doc->eln++;
	doc->els = realloc(doc->els, doc->eln * sizeof(bbcode *));
	doc->els[doc->eln - 1] = bb;
}

char *textcopy(char *text, size_t len)
{
	char *cpy;

	cpy = malloc(len + 1);
	memcpy(cpy, text, len);
	cpy[len] = '\0';

	return cpy;
}

size_t bbcode_fread(void *buf, size_t size, size_t nitems, void *stream)
{
	return fread(buf, size, nitems, (FILE *)stream);
}

size_t bbcode_fwrite(void *buf, size_t size, size_t nitems, void *stream)
{
	return fwrite(buf, size, nitems, (FILE *)stream);
}
