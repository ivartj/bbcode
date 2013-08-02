#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include "bbcode.h"

char *filename;
FILE *file;
bbcode_doc *m;

void usage(FILE *out)
{
	fprintf(out, "Usage: markup [ -h ] [ <markup-file> ]\n");
}

void parseargs(int argc, char *argv[])
{
	int c;
	static struct option longopts[] = {
		{ "help", no_argument, 0, 'h' },
		{ 0, 0, 0, 0 },
	};

	while((c = getopt_long(argc, argv, "h", longopts, NULL)) != -1)
	switch(c) {
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
	if(filename == NULL) {
		file = stdin;
		return;
	}

	file = fopen(filename, "r");
	if(file == NULL) {
		fprintf(stderr, "Failed to open file: %s.\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
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
	bbcode_print(m, bbcode_fwrite, stdout);
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
