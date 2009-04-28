/* $Id$ */
/*
 * Copyright (c) 2009 Dimitri Sokolyuk <sokolyuk@gmailcom>
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
#include <stdio.h>
#include <stdlib.h>

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

Cell *
readprog(char *fname)
{
	Cell *prog, **progp, *prev;
	FILE *fd;
	int ch;

	fd = fopen(fname, "r");
	assert(fd);

	prog = NULL;
	progp = &prog;
	prev = NULL;

	while ((ch = fgetc(fd)) != EOF) {
		switch (ch) {
		case '>':
		case '<':
		case '+':
		case '-':
		case ',':
		case '.':
		case '[':
		case ']':
			*progp = alloccell();
			(*progp)->value = ch;
			(*progp)->prev = prev;
			prev = *progp;
			progp = &(*progp)->next;
			break;
		default:
			break;
		}
	}

	fclose(fd);

	return prog;
}

int
main(int argc, char **argv)
{
	Cell *data, *prog, *p;
	int loop;

	if (argc != 2)
		return -1;

	prog = readprog(argv[1]);
	data = alloccell();

	for (p = prog; p; p = p->next) {
		switch (p->value) {
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
				for (loop = 0; p; p = p->next) {
					if (p->value == '[')
						loop++;
					if (p->value == ']' && --loop == 0)
						break;
				}
			break;
		case ']':
			if (data->value != 0)
				for (loop = 0; p; p = p->prev) {
					if (p->value == ']')
						loop++;
					if (p->value == '[' && --loop == 0)
						break;
				}
			break;
		}

		if (!p)
			break;
	}

	return 0;
}
