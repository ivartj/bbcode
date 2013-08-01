#include "bbcode.h"
#include <string.h>
#include "strut.h"
#include <stdlib.h>

typedef struct _ctx ctx;

struct _ctx {
	size_t (*read)(void *, size_t, size_t, void *);
	bbcode_doc *doc;
	void *data;
	char *buf;
	size_t len;
	size_t cap;
};

static int ctxgetc(ctx *x);
static void ctxadd(ctx *x, bbcode *bb);
static void parsebbcode(ctx *x);
static bbcode *parsetag(ctx *x, size_t off, size_t len);
static void ctxtext(ctx *x, size_t off, size_t len);

bbcode_doc *bbcode_parse(size_t (*read)(void *, size_t, size_t, void *), void *data)
{
	ctx x;

	memset(&x, 0, sizeof(x));

	x.read = read;
	x.data = data;

	x.doc = makedoc();

	parsebbcode(&x);

	return x.doc;
}

void parsebbcode(ctx *x)
{
	int c;
	int ti;
	int i;
	bbcode *bb;

	i = 0;
	while((c = ctxgetc(x)) != EOF) {
		switch(c) {
		case '[':
			ti = x->len - 1;
			break;
		case ']':
			if(ti < 0)
				break;
			bb = parsetag(x, ti + 1, x->len - ti - 2);
			if(bb == NULL)
				break;
			ctxtext(x, i, ti - i);
			ctxadd(x, bb);
			i = x->len;
			break;
		}
	}

	ctxtext(x, i, x->len - i);
}

bbcode *parsetag(ctx *x, size_t off, size_t len)
{
	char *buf;
	int stop;
	int type;

	if(len == 0)
		return NULL;

	type = 0;
	stop = 0;
	buf = x->buf + off;

	if(buf[0] == '/') {
		stop = 1;
		buf++;
		len--;
		if(len == 0)
			return NULL;
	}

	if(len == 1) {
		switch(buf[0]) {
		case 'B':
		case 'b':
			type = BBCODE_B;
			break;
		case 'I':
		case 'i':
			type = BBCODE_I;
			break;
		case 'U':
		case 'u':
			type = BBCODE_I;
			break;
		case 'S':
		case 's':
			type = BBCODE_S;
			break;
		default:
			return NULL;
		}
	} else if(len == 2)
		return NULL;
	else if(len == 3 && stop) {
		if(strncmp(buf, "url", 3) == 0 || strncmp(buf, "URL", 3) == 0) {
			type = BBCODE_URL;
		}
	} else if(len == 3)
		return NULL;
	else if(len >= 4) {
		if(strncmp(buf, "url=", 4) == 0 || strncmp(buf, "URL=", 3) == 0) {
			return makeurl(buf + 4, len - 4);
		}
	} else
		return NULL;


	if(stop)
		return makestop(type);

	switch(type) {
	case BBCODE_B:
		return makeb();
	case BBCODE_I:
		return makei();
	case BBCODE_U:
		return makeu();
	case BBCODE_S:
		return makes();
	default:
		return NULL;
	}
}

void ctxtext(ctx *x, size_t off, size_t len)
{
	docadd(x->doc, maketext(x->buf + off, len));
}

void ctxadd(ctx *x, bbcode *bb)
{
	docadd(x->doc, bb);
}

int ctxgetc(ctx *x)
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
	}

	x->buf[x->len++] = c;

	return c;
}
