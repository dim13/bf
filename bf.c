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

int
main(int argc, char **argv)
{
	FILE *fd;
	Cell *data, *prog, **progp, *p;
	int ch;

	if (argc != 2)
		return -1;

	fd = fopen(argv[1], "r");
	assert(fd);

	prog = NULL;
	progp = &prog;
	p = NULL;

	while ((ch = fgetc(fd)) != EOF) {
		*progp = alloccell();
		(*progp)->value = ch;
		(*progp)->prev = p;
		p = *progp;
		progp = &(*progp)->next;
	}

	fclose(fd);

	data = alloccell();

	for (p = prog; p; p = p->next)
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
				while (p && p->value != ']')
					p = p->next;
			break;
		case ']':
			if (data->value != 0)
				while (p && p->value != '[')
					p = p->prev;
			break;
		default:
			break;
		}

	return 0;
}
