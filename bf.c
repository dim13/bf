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

#if defined(__linux__)
#include <bsd/sys/queue.h>
#include <sys/types.h>
#else
#include <sys/queue.h>
#endif
#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct cell {
	int value;
	int number;
	TAILQ_ENTRY(cell) link;
} *dp, *dpn;

TAILQ_HEAD(cells, cell) data;

char *
readall(char *fname)
{
	int fd;
	char *buf;
	struct stat st;

	fd = open(fname, O_RDONLY);
	if (!fd)
		errx(1, "cannot open %s", fname);
	fstat(fd, &st);

	buf = calloc(st.st_size + 1, sizeof(char));
	if (!buf)
		errx(1, "calloc");

	read(fd, buf, st.st_size);
	close(fd);

	return buf;
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-d] <prog>\n", __progname);

	exit(1);
	/* NOTREACHED */
}

char *
locatejmp(char *p)
{
	for (; *p; p++)
		switch (*p) {
		case '[':
			p = locatejmp(p + 1);
			break;
		case ']':
			return p;
		default:
			break;
		}

	errx(1, "unbalanced loop");
	/* NOTREACHED */
}

struct cell *
alloccell(void)
{
	struct cell *c;

	c = calloc(1, sizeof(struct cell));
	if (!c)
		errx(1, "calloc");

	return c;
}

char *
execute(char *p)
{
	char *jmp;

	for (; *p; p++)
		switch (*p) {
		case '>':
			dpn = TAILQ_NEXT(dp, link);
			if (!dpn) {
				dpn = alloccell();
				dpn->number = dp->number + 1;
				TAILQ_INSERT_TAIL(&data, dpn, link);
			}
			dp = dpn;
			break;
		case '<':
			dpn = TAILQ_PREV(dp, cells, link);
			if (!dpn) {
				dpn = alloccell();
				dpn->number = dp->number - 1;
				TAILQ_INSERT_HEAD(&data, dpn, link);
			}
			dp = dpn;
			break;
		case '+':
			++dp->value;
			break;
		case '-':
			--dp->value;
			break;
		case '.':
			fputc(dp->value, stdout);
			fflush(stdout);
			break;
		case ',':
			dp->value = fgetc(stdin);
			break;
		case '[':
			if (!dp->value)
				jmp = locatejmp(p + 1);
			else while (dp->value)
				jmp = execute(p + 1);
			p = jmp;
			break;
		case ']':
			return p;
		default:
			break;
		}

	return p;
}

int
main(int argc, char **argv)
{
	char *prog;
	int c, dflag = 0;

	while ((c = getopt(argc, argv, "dh")) != -1)
		switch (c) {
		case 'd':
			dflag = 1;
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

	/* init first cell */
	TAILQ_INIT(&data);
	dp = alloccell();
	TAILQ_INSERT_HEAD(&data, dp, link);

	prog = readall(*argv);
	execute(prog);
	free(prog);

	/* dump cells */
	if (dflag)
		TAILQ_FOREACH(dp, &data, link)
			printf("%4d: 0x%.8x\n", dp->number, dp->value);

	/* free cells */
	TAILQ_FOREACH_SAFE(dp, &data, link, dpn)
		free(dp);

	return 0;
}
