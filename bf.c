/* $Id$ */
/*
 * Copyright (c) 2009 Dimitri Sokolyuk <demon@dim13.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct cell Cell;

struct cell {
	char value;
	Cell *next;
	Cell *prev;
};

Cell *
alloccell(void)
{
	Cell *c;

	c = malloc(sizeof(Cell));
	assert(c);

	c->value = 0;
	c->next = NULL;
	c->prev = NULL;

	return c;
}

void
freecells(Cell *c)
{
	Cell *next;

	while (c->prev)
		c = c->prev;
	
	while (c) {
		next = c->next;
		free(c);
		c = next;
	}
}

void
dumpcells(Cell *c)
{
	while (c->prev)
		c = c->prev;

	while (c) {
		printf("0x%-4.2x", c->value);
		c = c->next;
	}

	printf("\n");
}

char *
readprog(char *fname)
{
	char *prog, *p;
	FILE *fd;
	size_t len;
	int ch;

	fd = fopen(fname, "r");
	if (!fd)
		return NULL;

	fseek(fd, 0L, SEEK_END);
	len = ftell(fd);
	fseek(fd, 0L, SEEK_SET);

	prog = calloc(len + 1, sizeof(char));
	assert(prog);

	p = prog;
	while ((ch = fgetc(fd)) != EOF)
		if (strchr("><+-,.[]", ch))
			*p++ = ch;

	fclose(fd);

	return prog;
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-d] <prog>\n", __progname);

	exit(1);
}

int
main(int argc, char **argv)
{
	Cell *data;
	char *prog, *p;
	int dumpflag = 0;
	int loop;
	int c;

	while ((c = getopt(argc, argv, "dh")) != -1)
		switch (c) {
		case 'd':
			dumpflag = 1;
			break;
		case 'h':	
		case '?':	
		default:	
			usage();
			/* NOTREACHED */
		}

	argc -= optind;
	argv += optind;

	if (!argc)
		return -1;

	prog = readprog(*argv);
	if (!prog)
		errx(1, "not found: %s", *argv);

	data = alloccell();

	for (p = prog; *p; p++)
		switch (*p) {
		case '>':
			if (!data->next) {
				data->next = alloccell();
				data->next->prev = data;
			}
			data = data->next;
			break;
		case '<':
			if (!data->prev) {
				data->prev = alloccell();
				data->prev->next = data;
			}
			data = data->prev;
			break;
		case '+':
			++data->value;
			break;
		case '-':
			--data->value;
			break;
		case '.':
			fputc(data->value, stdout);
			fflush(stdout);
			break;
		case ',':
			data->value = fgetc(stdin);
			break;
		case '[':
			if (data->value == 0)
				for (loop = 0; *p; p++) {
					if (*p == '[')
						loop++;
					else if (*p == ']' && --loop == 0)
						break;
				}
			break;
		case ']':
			if (data->value != 0)
				for (loop = 0; *p; p--) {
					if (*p == ']')
						loop++;
					else if (*p == '[' && --loop == 0)
						break;
				}
			break;
		default:
			break;
		}

	if (dumpflag)
		dumpcells(data);

	freecells(data);
	free(prog);

	return 0;
}
