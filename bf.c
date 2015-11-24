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
#include <unistd.h>

char *
readall(char *fname, size_t *len)
{
	int fd;
	char *buf;
	struct stat st;

	fd = open(fname, O_RDONLY);
	if (!fd)
		return NULL;

	fstat(fd, &st);
	*len = st.st_size;

	buf = calloc(*len + 1, sizeof(char));

	read(fd, buf, *len);
	close(fd);

	return buf;
}

void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s <prog>\n", __progname);
	exit(1);
	/* NOTREACHED */
}

char *
mkjmptbl(char *prog, char **jmp)
{
	char *to;

	for (; *prog; prog++, jmp++) {
		switch (*prog) {
		case '[':
			*jmp = to = mkjmptbl(prog + 1, jmp + 1);
			jmp += to - prog;
			*jmp = prog;
			prog = to;
			break;
		case ']':
			return prog;
		default:
			break;
		}
	}

	return NULL;
}

void
execute(char *data, char *prog, char **jmp, size_t sz)
{
	char *pc = prog;
	char *dp = data + sz / 4;

	for (; *pc; pc++)
		switch (*pc) {
		case '>':
			if (dp - data == sz)
				errx(1, "memory overflow");
			++dp;
			break;
		case '<':
			if (dp - data == 0)
				errx(1, "memory underflow");
			--dp;
			break;
		case '+':
			++*dp;
			break;
		case '-':
			--*dp;
			break;
		case '.':
			fputc(*dp, stdout);
			break;
		case ',':
			*dp = fgetc(stdin);
			break;
		case '[':
			if (*dp == 0)
				pc = jmp[pc - prog];
			break;
		case ']':
			if (*dp != 0)
				pc = jmp[pc - prog];
			break;
		default:
			break;
		}
}

int
main(int argc, char **argv)
{
	char **jmp;
	char *prog;
	char *data;
	size_t datasz;
	size_t len;

	argc--;
	argv++;

	if (!argc)
		usage();
		/* NOTREACHED */

	prog = readall(*argv, &len);
	if (!prog)
		errx(1, "cannot open %s", *argv);

	datasz = getpagesize();
	data = calloc(datasz, sizeof(char));
	jmp = calloc(len, sizeof(char *));

	mkjmptbl(prog, jmp);
	execute(data, prog, jmp, datasz);

	free(data);
	free(prog);
	free(jmp);

	return 0;
}
