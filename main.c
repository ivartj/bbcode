#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "bbcode.h"

char *filename = NULL;
FILE *file = NULL;
char *outfile = NULL;
FILE *out = NULL;
bbcode_doc *m = NULL;
char *rootpath = NULL;

void usage(FILE *out)
{
	fprintf(out, "usage: bbcode [ -o <output-file> ] [ <input-file> ]\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "out", required_argument, 0, 'o' },
		{ "help", no_argument, 0, 'h' },
		{ "root-path", required_argument, NULL, 'r' }, 
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "ho:r:", longopts, NULL)) != -1)
	switch(c) {
	case 'o':
		outfile = optarg;
		break;
	case 'r':
		rootpath = optarg;
		break;
	case 'h':
		usage(stdout);
		exit(EXIT_SUCCESS);
	case '?':
		usage(stderr);
		exit(EXIT_FAILURE);
	}

	switch(argc - optind) {
	case 0:
		filename == NULL;
		break;
	case 1:
		filename = argv[optind];
		break;
	default:
		usage(stderr);
		exit(EXIT_FAILURE);
	}

}

void openfile(void)
{
	if(filename != NULL) {
		file = fopen(filename, "r");
		if(file == NULL) {
			fprintf(stderr, "Failed to open file: %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else
		file = stdin;

	if(outfile != NULL) {
		out = fopen(outfile, "w");
		if(out == NULL) {
			fprintf(stderr, "Failed to open file for writing: %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}
	} else
		out = stdout;

}

void closefile(void)
{
	fclose(file);
}

void parse(void)
{
	m = bbcode_parse(bbcode_fread, file);
	if(m == NULL) {
		fprintf(stderr, "Failed to parse input.\n");
		exit(EXIT_FAILURE);
	}
}

void print(void)
{
	bbcode_print(m, bbcode_fwrite, out, rootpath);
}

int main(int argc, char *argv[])
{
	parseargs(argc, argv);
	openfile();
	atexit(closefile);
	parse();
	print();
	exit(EXIT_SUCCESS);
}
