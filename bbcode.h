#ifndef BBCODE_H
#define BBCODE_H

#include <stdio.h>

typedef union bbcode bbcode;
typedef struct bbcode_text bbcode_text;
typedef struct bbcode_b bbcode_b;
typedef struct bbcode_i bbcode_i;
typedef struct bbcode_u bbcode_u;
typedef struct bbcode_s bbcode_s;
typedef struct bbcode_url bbcode_url;
typedef struct bbcode_img bbcode_img;
typedef struct bbcode_doc bbcode_doc;
typedef struct bbcode_stop bbcode_stop;

#define BBCODE_TEXT	1
#define BBCODE_B	2
#define BBCODE_I	3
#define BBCODE_U	4
#define BBCODE_S	5
#define BBCODE_URL	6
#define BBCODE_URL_STOP	7
#define BBCODE_IMG	8
#define BBCODE_STOP	9
#define BBCODE_DOC	10
#define BBCODE_NUMTYPES 11

struct bbcode_text {
	int type;
	char *text;
	size_t len;
};

struct bbcode_b {
	int type;
};

struct bbcode_i {
	int type;
};

struct bbcode_u {
	int type;
};

struct bbcode_s {
	int type;
};

struct bbcode_url {
	int type;
	char *url;
	size_t len;
};

struct bbcode_img {
	int type;
	char *url;
	size_t urllen;
};

struct bbcode_doc {
	int type;
	int eln;
	bbcode **els;
};

struct bbcode_stop {
	int type;
	int stoptype;
};

union bbcode {
	int type;
	bbcode_text text;
	bbcode_b b;
	bbcode_i i;
	bbcode_u u;
	bbcode_s s;
	bbcode_url url;
	bbcode_img img;
	bbcode_doc doc;
	bbcode_stop stop;
};


/* returns NULL on failure to parse */
bbcode_doc *bbcode_parse(size_t (*read)(void *, size_t, size_t, void *), void *data);

/* returns how much written or negative number if error */
int bbcode_print_html(size_t (*write)(void *, size_t, size_t, void*), void *data, bbcode_doc *bb);

size_t bbcode_fwrite(void *ptr, size_t size, size_t nitems, void *stream);
size_t bbcode_fread(void *ptr, size_t size, size_t nitems, void *stream);

#endif
