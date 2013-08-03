#include "bbcode.h"
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#define BBCODE_INLINE	1
#define BBCODE_BLOCK	2
#define BBCODE_CONTENT	3

#define LENGTH(arr)	(sizeof(arr) / sizeof(*arr))
#define STRING(str)	str, sizeof(str) - 1

typedef struct _dlist dlist;
typedef struct _bbcode bbcode;
typedef struct _bbcode_type bbcode_type;
typedef struct _rx rx;
typedef struct _px px;
typedef int (*printfn)(px *x, bbcode *bb);

struct _bbcode_doc {
	size_t len;
	size_t cap;
	bbcode **els;
	char *src;
	size_t srclen;
};

struct _bbcode {
	bbcode_type *type;
	int stop;
	int match;
	size_t srcoff;
	size_t srclen;
	size_t paramoff;
	size_t paramlen;
};

struct _bbcode_type {
	char *name;
	size_t namelen;
	int type;
	printfn print;
	void *printdata;
};

struct _rx {
	size_t (*read)(void *, size_t, size_t, void *);
	bbcode_doc *doc;
	void *data;
	char *buf;
	size_t len;
	size_t cap;
};

struct _px {
	bbcode_doc *doc;
	size_t (*write)(void *, size_t, size_t, void *);
	void *data;
	dlist *first;
	dlist *last;
};

struct _dlist {
	bbcode *bb;
	dlist *next;
	dlist *prev;
};

static int xprintf(px *x, const char *fmt, ...);
static int xputc(px *x, char c);
static int xgetc(rx *x);
static void parsebb(rx *x);
static int printhtml(px *x);
static void addbb(bbcode_doc *doc, bbcode *bb);
static void addtag(bbcode_doc *doc, bbcode *bb);
static void addtext(bbcode_doc *doc, char *text, size_t len);
static bbcode *parsetag(rx *x, char *text, size_t len);
static void findmatch(bbcode_doc *doc, bbcode *bb);
static int printhtmltext(px *x, bbcode *bb);
static int printhtmltag(px *x, bbcode *bb);
static int printhtmlimg(px *x, bbcode *bb);
static int printhtmlurl(px *x, bbcode *bb);
static int printbb(px *x, bbcode *bb);
static int xstack(px *x, bbcode *b);
static int xuntangle(px *x, bbcode *b);
static int ismatch(bbcode *start, bbcode *stop);
static int xunwind(px *x);
static int xrewind(px *x);

#define BBCODE_TEXT	0
#define BBCODE_B	1
#define BBCODE_I	2
#define BBCODE_U	3
#define BBCODE_S	4
#define BBCODE_URL	5
#define BBCODE_QUOTE	6
#define BBCODE_IMG	7

static bbcode_type bbtypes[] = {
	[BBCODE_TEXT]	= { NULL, 0, BBCODE_CONTENT, printhtmltext, NULL },
	[BBCODE_B]	= { STRING("b"), BBCODE_INLINE, printhtmltag, "strong" },
	[BBCODE_I]	= { STRING("i"), BBCODE_INLINE, printhtmltag, "em" },
	[BBCODE_U]	= { STRING("u"), BBCODE_INLINE, printhtmltag, "u" },
	[BBCODE_S]	= { STRING("s"), BBCODE_INLINE, printhtmltag, "stroke" },
	[BBCODE_URL]	= { STRING("url"), BBCODE_INLINE, printhtmlurl, NULL },
	[BBCODE_QUOTE]	= { STRING("quote"), BBCODE_BLOCK, printhtmltag, "blockquote" },
	[BBCODE_IMG]	= { STRING("img"), BBCODE_CONTENT, printhtmlimg, NULL },
};

size_t bbcode_fwrite(void *ptr, size_t size, size_t nitems, void *stream)
{
	return fwrite(ptr, size, nitems, (FILE *)stream);
}

size_t bbcode_fread(void *ptr, size_t size, size_t nitems, void *stream)
{
	return fread(ptr, size, nitems, (FILE *)stream);
}

int xprintf(px *x, const char *fmt, ...)
{
	char *str;
	size_t size;
	va_list ap, apcpy;

	va_start(ap, fmt);
	va_copy(apcpy, ap);
	size = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	str = malloc(size + 1);
	vsnprintf(str, size + 1, fmt, apcpy);
	va_end(apcpy);

	size = x->write(str, 1, size, x->data);
	free(str);

	return size;
}

int xputc(px *x, char c)
{
	return x->write(&c, 1, 1, x->data);
}

int xgetc(rx *x)
{
	int size;
	char c;

	size = 0;
	size = x->read(&c, 1, 1, x->data);
	if(size == 0)
		return EOF;

	if(x->len == x->cap) {
		if(x->cap == 0)
			x->cap = 256;
		else
			x->cap *= 2;
		x->buf = realloc(x->buf, x->cap);
		x->doc->src = x->buf;
	}

	x->buf[x->len++] = c;
	x->doc->srclen = x->len;

	return c;
}

bbcode_doc *bbcode_parse(size_t (*read)(void *, size_t, size_t, void *), void *data)
{
	rx x = { 0 };
	bbcode *bb;

	x.read = read;
	x.data = data;
	x.doc = calloc(1, sizeof(bbcode_doc));

	parsebb(&x);

	return x.doc;
}

void parsebb(rx *x)
{
	int c;
	int ti;
	bbcode *bb;
	int i;

	ti = -1;
	i = 0;

	while((c = xgetc(x)) != EOF)
	switch(c) {
	case '[':
		ti = x->len - 1;
		break;
	case ']':
		if(ti < 0)
			break;
		bb = parsetag(x, x->buf + ti, x->len - ti);
		if(bb == NULL)
			break;
		addtext(x->doc, x->buf + i, ti - i);
		addtag(x->doc, bb);
		i = x->len;
		break;
	}

	addtext(x->doc, x->buf + i, x->len - i);
}

void addbb(bbcode_doc *doc, bbcode *bb)
{
	if(doc->len == doc->cap) {
		if(doc->len == 0)
			doc->cap = 256;
		else
			doc->cap *= 2;
		doc->els = realloc(doc->els, sizeof(bbcode *) * doc->cap);
	}

	doc->els[doc->len++] = bb;
}

void addtag(bbcode_doc *doc, bbcode *bb)
{
	if(!bb->stop) {
		addbb(doc, bb);
		return;
	}

	findmatch(doc, bb);

	addbb(doc, bb);
}

// TODO: Deal with block nesting level
void findmatch(bbcode_doc *doc, bbcode *bb)
{
	int i;
	bbcode *cand;

	for(i = doc->len - 1; i >= 0; i--) {
		cand = doc->els[i];
		if(cand->stop)
			continue;
		if(cand->type != bb->type)
			continue;
		if(cand->match)
			continue;
		break;
	}
	if(i == -1)
		return;

	bb->match = 1;
	cand->match = 1;
}

void addtext(bbcode_doc *doc, char *text, size_t len)
{
	bbcode *bb;

	bb = calloc(1, sizeof(bbcode));
	bb->type = BBCODE_TEXT;
	bb->srcoff = text - doc->src;
	bb->srclen = len;

	addbb(doc, bb);
}

bbcode *parsetag(rx *x, char *text, size_t len)
{
	int i;
	char *name;
	size_t namelen;
	bbcode_type *type;
	bbcode *bb;
	int param;
	int stop;

	name = text + 1;
	namelen = len - 2;
	param = 0;
	stop = 0;

	if(namelen >= 1)
	if(name[0] == '/') {
		stop = 1;
		name++;
		namelen--;
	}

	for(i = 0; i < LENGTH(bbtypes); i++) {
		type = &(bbtypes[i]);
		if(type->namelen > namelen)
			continue;
		if(strncmp(type->name, name, type->namelen) != 0)
			continue;
		if(namelen == type->namelen)
			break;
		if(stop)
			continue;
		if(name[type->namelen] == '=') {
			param = 1;
			break;
		}
	}

	if(i == LENGTH(bbtypes)) {
		return NULL;
	}

	bb = calloc(1, sizeof(bbcode));
	bb->type = type;
	bb->srcoff = text - x->doc->src;
	bb->srclen = len;
	bb->stop = stop;
	if(param) {
		bb->paramoff = text - x->buf + 1 + type->namelen + 1;
		bb->paramlen = len - 3 - type->namelen;
	}

	return bb;
}

int bbcode_print(bbcode_doc *doc, size_t (*write)(void *, size_t, size_t, void*), void *data)
{
	px x = { 0 };
	int n;

	x.write = write;
	x.data = data;
	x.doc = doc;

	n = printhtml(&x);

	return n;
}

int printhtml(px *x)
{
	int i;
	bbcode *bb;
	int n;
	bbcode_doc *doc;

	n = 0;

	doc = x->doc;

	for(i = 0; i < doc->len; i++) {
		bb = doc->els[i];

		n += printbb(x, bb);
	}

	return n;
}

int printstart(px *x, bbcode *bb)
{
	int tmp;
	int n;

	tmp = bb->stop;
	bb->stop = 0;
	n = bb->type->print(x, bb);
	bb->stop = tmp;

	return n;
}

int printstop(px *x, bbcode *bb)
{
	int tmp;
	int n;

	tmp = bb->stop;
	bb->stop = 1;
	n = bb->type->print(x, bb);
	bb->stop = tmp;

	return n;
}

int printhtmltext(px *x, bbcode *bb)
{
	int i;
	char c;
	int n;
	char *src;

	n = 0;
	src = x->doc->src + bb->srcoff;

	for(i = 0; i < bb->srclen; i++) {
		c = src[i];
		switch(c) {
		case '\n':
			n += xprintf(x, "<br />\n");
			break;
		case '&':
			n += xprintf(x, "&amp;");
			break;
		case '<':
			n += xprintf(x, "&lt;");
			break;
		case '>':
			n += xprintf(x, "&gt;");
			break;
		default:
			n += xputc(x, c);
		}
	}
}

int printhtmltag(px *x, bbcode *bb)
{
	char *tagname;

	assert(bb->match);

	tagname = bb->type->printdata;
	if(bb->stop)
		return xprintf(x, "</%s>", tagname);
	else
		return xprintf(x, "<%s>", tagname);
}

int printhtmlimg(px *x, bbcode *bb)
{
	return 0;
}

int xstack(px *x, bbcode *bb)
{
	dlist *l;
	int n;

	n = 0;

	if(bb->type->type == BBCODE_BLOCK)
		n += xunwind(x);

	n += bb->type->print(x, bb);

	if(bb->type->type == BBCODE_BLOCK)
		n += xrewind(x);

	if(bb->type->type != BBCODE_INLINE)
		return n;

	l = calloc(1, sizeof(dlist));
	l->bb = bb;
	l->next = x->last;
	x->last = l;
	if(x->first == NULL)
		x->first = l;
	else
		l->next->prev = l;

	return n;
}

int ismatch(bbcode *start, bbcode *stop)
{
	if(start->stop)
		return 0;
	if(!stop->stop)
		return 0;
	if(start->type != stop->type)
		return 0;

	return 1;
}

int xuntangle(px *x, bbcode *bb)
{
	dlist *l, *s;
	int n;

	if(bb->type->type != BBCODE_INLINE)
		return bb->type->print(x, bb);

	n = 0;

	for(l = x->last; !ismatch(l->bb, bb); l = l->next)
		n += printstop(x, l->bb);

	s = l;
	n += printstop(x, s->bb);

	for(l = l->prev; l != NULL; l = l->prev)
		n += printstart(x, l->bb);

	if(s->next != NULL)
		s->next->prev = s->prev;
	else
		x->first = s->prev;

	if(s->prev != NULL)
		s->prev->next = s->next;
	else
		x->last = s->next;

	free(s);
}

int printbb(px *x, bbcode *bb)
{
	if(bb->match) {
		if(!bb->stop)
			return xstack(x, bb);
		else
			return xuntangle(x, bb);
	} else
		return printhtmltext(x, bb);

}

static int xunwind(px *x)
{
	dlist *l;
	int n;

	n = 0;
	for(l = x->last; l != NULL; l = l->next)
		n += printstop(x, l->bb);

	return n;
}

static int xrewind(px *x)
{
	dlist *l;
	int n;

	n = 0;
	for(l = x->first; l != NULL; l = l->prev)
		n += printstart(x, l->bb);

	return n;
}

int printhtmlattribute(px *x, char *text, size_t len)
{
	int i;
	char c;
	int n;

	n = 0;
	for(i = 0; i < len; i++) {
		c = text[i];
		switch(c) {
		case '"':
			n += xprintf(x, "&quot;");
			break;
		case '&':
			n += xprintf(x, "&amp;");
			break;
		case '<':
			n += xprintf(x, "&lt;");
			break;
		case '>':
			n += xprintf(x, "&gt;");
			break;
		default:
			n + xputc(x, c);
		}
	}

	return n;
}

int printhtmlurl(px *x, bbcode *bb)
{
	int n;

	if(bb->stop)
		return xprintf(x, "</a>");

	if(bb->paramlen == 0)
		return xprintf(x, "<a href=\"#\">");

	n = 0;

	n += xprintf(x, "<a href=\"");
	n += printhtmlattribute(x, x->doc->src + bb->paramoff, bb->paramlen);
	n += xprintf(x, "\">");

	return n;
}

void bbcode_doc_destroy(bbcode_doc *doc)
{
	int i;

	for(i = 0; i < doc->len; i++)
		free(doc->els[i]);

	free(doc->src);
	free(doc);
}
