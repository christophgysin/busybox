/* vi: set sw=4 ts=4: */
/*
 * Mini wc implementation for busybox
 *
 * Copyright (C) 2000  Edward Betts <edward@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include "busybox.h"

static int total_lines, total_words, total_chars, max_length;
//static int print_lines, print_words, print_chars, print_length;
static char print_type = 0;
enum print_e {
	print_lines = 1,
	print_words = 2,
	print_chars = 4,
	print_length = 8
};

static void print_counts(int lines, int words, int chars, int length,
				  		 const char *name)
{
	if (print_type & print_lines) {
		printf("%7d ", lines);
	}
	if (print_type & print_words) {
		printf("%7d ", words);
	}
	if (print_type & print_chars) {
		printf("%7d ", chars);
	}
	if (print_type & print_length) {
		printf("%7d ", length);
	}
	if (*name) {
		printf("%s", name);
	}
	putchar('\n');
}

static void wc_file(FILE * file, const char *name)
{
	int lines, words, chars, length;
	int in_word = 0, linepos = 0;
	int c;

	lines = words = chars = length = 0;
	while ((c = getc(file)) != EOF) {
		chars++;
		switch (c) {
		case '\n':
			lines++;
		case '\r':
		case '\f':
			if (linepos > length)
				length = linepos;
			linepos = 0;
			goto word_separator;
		case '\t':
			linepos += 8 - (linepos % 8);
			goto word_separator;
		case ' ':
			linepos++;
		case '\v':
		  word_separator:
			if (in_word) {
				in_word = 0;
				words++;
			}
			break;
		default:
			linepos++;
			in_word = 1;
			break;
		}
	}
	if (linepos > length)
		length = linepos;
	if (in_word)
		words++;
	print_counts(lines, words, chars, length, name);
	total_lines += lines;
	total_words += words;
	total_chars += chars;
	if (length > max_length)
		max_length = length;
	fclose(file);
	fflush(stdout);
}

int wc_main(int argc, char **argv)
{
	FILE *file;
	unsigned int num_files_counted = 0;
	int opt, status = EXIT_SUCCESS;

	total_lines = total_words = total_chars = max_length = 0;

	while ((opt = getopt(argc, argv, "clLw")) > 0) {
			switch (opt) {
			case 'c':
				print_type |= print_chars;
				break;
			case 'l':
				print_type |= print_lines;
				break;
			case 'L':
				print_type |= print_length;
				break;
			case 'w':
				print_type |= print_words;
				break;
			default:
				show_usage();
			}
	}

	if (print_type == 0) {
		print_type = print_lines | print_words | print_chars;
	}

	if (argv[optind] == NULL || strcmp(argv[optind], "-") == 0) {
		wc_file(stdin, NULL);
		return EXIT_SUCCESS;
	} else {
		while (optind < argc) {
			file = wfopen(argv[optind], "r");
			if (file != NULL)
				wc_file(file, argv[optind]);
			else
				status = EXIT_FAILURE;
			num_files_counted++;
			optind++;
		}
	}

	if (num_files_counted > 1)
		print_counts(total_lines, total_words, total_chars,
					 max_length, "total");

	return status;
}
