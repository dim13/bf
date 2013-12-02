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

	c = calloc(1, sizeof(Cell));
	if (!c)
		errx(1, "calloc");

	return c;
}

Cell *
rewindcells(Cell *c)
{
	while (c->prev)
		c = c->prev;

	return c;
}

void
freecells(Cell *c)
{
	Cell *next;

	for (c = rewindcells(c); c; c = next) {
		next = c->next;
		free(c);
	}
}

void
dumpcells(Cell *c)
{
	for (c = rewindcells(c); c; c = c->next)
		printf("0x%x\n", c->value);
}

char *
freadall(char *fname)
{
	FILE *fd;
	char *buf;
	size_t len;

	fd = fopen(fname, "r");
	if (!fd)
		errx(1, "cannot open %s", fname);

	fseek(fd, 0L, SEEK_END);
	len = ftell(fd);
	fseek(fd, 0L, SEEK_SET);

	buf = calloc(len + 1, sizeof(char));
	if (!buf)
		errx(1, "calloc");

	fread(buf, sizeof(char), len, fd);
	fclose(fd);

	return buf;
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-d] <prog>\n", __progname);

	exit(1);
}

char *
locatejmp(char *prog)
{
	for (; *prog; prog++)
		switch (*prog) {
		case '[':
			prog = locatejmp(prog + 1);
			break;
		case ']':
			return prog;
		default:
			break;
		}

	errx(1, "unbalanced loop");
	/* NOTREACHED */
}

Cell *
execute(Cell *data, char *prog)
{
	for (; *prog; prog++)
		switch (*prog) {
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
			while (data->value)
				data = execute(data, prog + 1);
			prog = locatejmp(prog + 1);
			break;
		case ']':
			return data;
		default:
			break;
		}

	return data;
}

int
main(int argc, char **argv)
{
	Cell *data;
	char *prog;
	int dumpflag = 0;
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
		errx(1, "no argument");

	prog = freadall(*argv);
	data = alloccell();
	execute(data, prog);

	if (dumpflag)
		dumpcells(data);

	freecells(data);
	free(prog);

	return 0;
}
