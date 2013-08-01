#include "bbcode.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

typedef struct _ctx ctx;
typedef struct _dlist dlist;

struct _ctx {
	dlist *first;
	dlist *last;
	size_t (*write)(void *, size_t, size_t, void *);
	void *data;
};

struct _dlist {
	bbcode *bb;
	dlist *next;
	dlist *prev;
};

static int printhtmltext(ctx *x, bbcode *b);
static int printhtmlb(ctx *x, bbcode *b);
static int printhtmli(ctx *x, bbcode *b);
static int printhtmlu(ctx *x, bbcode *b);
static int printhtmls(ctx *x, bbcode *b);
static int printhtmlurl(ctx *x, bbcode *b);
static int printhtmlimg(ctx *x, bbcode *b);
static int printhtmlstop(ctx *x, bbcode *b);
static int printhtmlattrib(ctx *x, char *text, size_t len);
static int printhtmldoc(ctx *x, bbcode *b);
static int ctxprintf(ctx *x, const char *fmt, ...);
static int ctxputc(ctx *x, char c);
static void ctxstack(ctx *x, bbcode *b);
static int printhtmlend(ctx *x, int type); 

static int (*printhtmlbbcode[BBCODE_NUMTYPES])(ctx *x, bbcode *bb) = {
	[BBCODE_TEXT]		= printhtmltext,
	[BBCODE_B]		= printhtmlb,
	[BBCODE_I]		= printhtmli,
	[BBCODE_U]		= printhtmlu,
	[BBCODE_S]		= printhtmls,
	[BBCODE_URL]		= printhtmlurl,
	[BBCODE_IMG]		= printhtmlimg,
	[BBCODE_STOP]		= printhtmlstop,
	[BBCODE_DOC]		= printhtmldoc,
};

int bbcode_print_html(size_t (*write)(void *, size_t, size_t, void*), void *data, bbcode_doc *bb)
{
	ctx x;

	memset(&x, 0, sizeof(x));
	if(write != NULL)
		x.write = write;
	else
		x.write = bbcode_fwrite;
	if(data != NULL)
		x.data = data;
	else
		x.data = stdout;

	return printhtmldoc(&x, (bbcode *)bb);
}

int printhtmldoc(ctx *x, bbcode *b)
{
	bbcode_doc *doc;
	int i;
	int size;
	bbcode *el;
	dlist *l;

	size = 0;
	doc = &(b->doc);
	for(i = 0; i < doc->eln; i++) {
		el = doc->els[i];
		size += printhtmlbbcode[el->type](x, el);
		ctxstack(x, el);
	}

	for(l = x->last; l != NULL; l = l->next)
		size += printhtmlend(x, l->bb->type);

	return size;
}

int printhtmltext(ctx *x, bbcode *b)
{
	bbcode_text *t;
	int i;
	char c;
	int size;

	t = &(b->text);

	for(i = 0; i < t->len; i++) {
		c = t->text[i];
		switch(c) {
		case '<':
			size += ctxprintf(x, "&lt;");
			break;
		case '>':
			size += ctxprintf(x, "&gt;");
			break;
		case '&':
			size += ctxprintf(x, "&amp;");
			break;
		case '\n':
			size += ctxprintf(x, "<br>\n");
			break;
		case ' ':
			size += ctxputc(x, c); /* TODO */
			break;
		case '\t':
			size += ctxprintf(x, "&nbsp; &nbsp;&nbsp; &nbsp; &nbsp");
			break;
		default:
			size += ctxputc(x, c);
			break;
		}
	}

	return size;
}

int printhtmlb(ctx *x, bbcode *b)
{
	return ctxprintf(x, "<strong>");
}

int printhtmli(ctx *x, bbcode *b)
{
	return ctxprintf(x, "<em>");
}

int printhtmlu(ctx *x, bbcode *b)
{
	return ctxprintf(x, "<i>");
}

int printhtmls(ctx *x, bbcode *b)
{
	return ctxprintf(x, "<stroke>");
}

int printhtmlurl(ctx *x, bbcode *b)
{
	int size;
	int i;
	bbcode_url *url;
	char c;

	url = &(b->url);
	size = 0;
	size += ctxprintf(x, "<a href=\"");
	size += printhtmlattrib(x, url->url, url->len);
	size += ctxprintf(x, "\">");

	return size;
}

int printhtmlimg(ctx *x, bbcode *b)
{
	int size;
	bbcode_img *img;

	img = &(b->img);
	size = 0;
	size += ctxprintf(x, "<img alt=\"\" src=\"");
	size += printhtmlattrib(x, img->url, img->urllen);
	size += ctxprintf(x, "\">");

	return size;
}

static int printhtmlend(ctx *x, int type)
{
	switch(type) {
	case BBCODE_B:
		return ctxprintf(x, "</strong>");
	case BBCODE_I:
		return ctxprintf(x, "</em>");
	case BBCODE_U:
		return ctxprintf(x, "</u>");
	case BBCODE_S:
		return ctxprintf(x, "</stroke>");
	case BBCODE_URL:
		return ctxprintf(x, "</a>");
	default:
		return 0;
	}
}

int printhtmlstop(ctx *x, bbcode *b)
{
	dlist *l, *s;
	bbcode_stop *stop;
	int size;

	size = 0;

	stop = &(b->stop);

	for(s = x->last; s != NULL; s = s->next)
		if(s->bb->type == stop->stoptype)
			break;
	if(s == NULL)
		return 0;

	for(l = x->last; l != s; l = l->next)
		size += printhtmlend(x, l->bb->type);
	size += printhtmlend(x, s->bb->type);
	for(l = s->prev; l != NULL; l = l->prev) {
		size += printhtmlbbcode[l->bb->type](x, l->bb);
		break;
	}

	if(s->next != NULL)
		s->next->prev = s->prev;
	else
		x->first = s->prev;

	if(s->prev != NULL)
		s->prev->next = s->next;
	else
		x->last = s->next;

	free(s);

	return size;
}

int ctxprintf(ctx *x, const char *fmt, ...)
{
	char *str;
	size_t size;
	va_list ap, apcpy;

	va_start(ap, fmt);
	va_copy(apcpy, ap);
	size = vsnprintf(NULL, 0, fmt, ap);
	va_end(apcpy);

	str = malloc(size + 1);
	vsnprintf(str, size + 1, fmt, apcpy);
	va_end(apcpy);

	size = x->write(str, 1, size, x->data);
	free(str);

	return size;
}

int ctxputc(ctx *x, char c)
{
	return x->write(&c, 1, 1, x->data);
}

void ctxstack(ctx *x, bbcode *b)
{
	dlist *l;

	switch(b->type) {
	case BBCODE_B:
	case BBCODE_I:
	case BBCODE_U:
	case BBCODE_S:
	case BBCODE_URL:
		break;
	default:
		return;
	}

	l = calloc(1, sizeof(dlist));
	l->bb = b;
	l->next = x->last;
	x->last = l;
	if(x->first == NULL)
		x->first = l;
	else
		l->next->prev = l;
}

int printhtmlattrib(ctx *x, char *text, size_t len)
{
	int size;
	int i;
	char c;

	size = 0;
	for(i = 0; i < len; i++) {
		c = text[i];
		switch(c) {
		case '<':
			size += ctxprintf(x, "&lt;");
			break;
		case '>':
			size += ctxprintf(x, "&gt;");
			break;
		case '&':
			size += ctxprintf(x, "&amp;");
			break;
		case '"':
			size += ctxprintf(x, "&quot;");
			break;
		case '\'':
			size += ctxprintf(x, "&#39;");
			break;
		default:
			size += ctxputc(x, c);
			break;
		}
	}

	return size;
}
