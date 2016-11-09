/*
 * Main functions of the program.
 *
 * Copyright (C) 2007 David Härdeman <david@hardeman.nu>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; only version 2 of the License is applicable.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <grp.h>
#include <pwd.h>

#include "utils.h"

/* Controls the verbosity level for msg() */
static int verbosity = 0;

/* Adjusts the verbosity level for msg() */
void
adjust_verbosity(int adj)
{
	verbosity += adj;
}

/*
 * Prints messages to console according to the current verbosity
 * - see utils.h for level defines
 */
int
msg(int level, const char *fmt, ...)
{
	int ret;
	va_list ap;

	if (level > verbosity)
		return 0;

	va_start(ap, fmt);

	if (level < MSG_QUIET)
		ret = vfprintf(stderr, fmt, ap);
	else
		ret = vfprintf(stdout, fmt, ap);

	va_end(ap);
	return ret;
}

/* Malloc which either succeeds or exits */
void *
xmalloc(size_t size)
{
        void *result = malloc(size);
        if (!result) {
                msg(MSG_CRITICAL, "Failed to malloc %zi bytes\n", size);
                exit(EXIT_FAILURE);
        }
        return result;
}

/* Ditto for strdup */
char *
xstrdup(const char *s)
{
	char *result = strdup(s);
	if (!result) {
		msg(MSG_CRITICAL, "Failed to strdup %zi bytes\n", strlen(s));
		exit(EXIT_FAILURE);
	}
	return result;
}

/* Human-readable printout of binary data */
void
binary_print(const char *s, ssize_t len)
{
	ssize_t i;

	for (i = 0; i < len; i++) {
		if (isprint(s[i]))
			msg(MSG_DEBUG, "%c", s[i]);
		else
			msg(MSG_DEBUG, "0x%02X", (int)s[i]);
	}
}

/* Writes data to a file or exits on failure */
void
xfwrite(const void *ptr, size_t size, FILE *stream)
{
	if (size && fwrite(ptr, size, 1, stream) != 1) {
		msg(MSG_CRITICAL, "Failed to write to file: %s\n",
		    strerror(errno));
		exit(EXIT_FAILURE);
	}
}

/* Writes an int to a file, using len bytes, in bigendian order, with separator */
void
write_int(uint64_t value, FILE *to)
{
	char buf[32]; // base 10 string representation of long long unsigned int has at max 20 chars
	int result;

	result = sprintf(buf, "%llu%c", (long long unsigned int)value, SEPARATOR);
	if (result < 0)
		msg(MSG_CRITICAL, "Integer could not be converted to string\n");
	xfwrite(buf, strlen(buf), to);
}

/* Writes a binary string to a file */
void
write_binary_string(const char *string, size_t len, FILE *to)
{
	xfwrite(string, len, to);
}

/* Writes a normal C string to a file, with separator */
void
write_string(const char *string, FILE *to)
{
	char sep[1];

	*sep = SEPARATOR;
	xfwrite(string, strlen(string), to);
	xfwrite(sep, 1, to);
}

/* Reads an int from a file, using len characters, in bigendian order, with separator */
uint64_t
read_int(char **from, const char *max)
{
	uint64_t result = 0;
	char *endptr, *separator;

	separator = strchr(*from, SEPARATOR);
	if (separator == NULL || separator > max) {
		msg(MSG_CRITICAL,
		    "Attempt to read integer beyond end of file, corrupt file?\n");
		exit(EXIT_FAILURE);
	}
	result = (uint64_t) strtoull(*from,&endptr,10);

	if (errno == ERANGE)
		msg(MSG_CRITICAL, "String could not be converted to integer\n");

	if (result == 0ULL && endptr != separator)
		msg(MSG_CRITICAL, "Integer could not be read from file\n");

	endptr++;
	*from = endptr;
	return result;
}

/* Reads a normal C string from a file */
char *
read_binary_string(char **from, size_t len, const char *max)
{
	char *result;

	if (*from + len > max) {
		msg(MSG_CRITICAL,
		    "Attempt to read string beyond end of file, corrupt file?\n");
		exit(EXIT_FAILURE);
	}
	result = xmalloc(len + 1);
	memcpy(result, *from, len);
	*from += len;
	return result;
}

/* Reads a normal C string from a file with separator*/
char *
read_string(char **from, const char *max)
{
	char *separator, *result;
	size_t len;

	separator = strchr(*from, SEPARATOR);
	len = separator - *from;
	result = read_binary_string(from, len, max);
	(*from)++;
	return result;
}

/* For group caching */
static struct group *gtable = NULL;

/* Initial setup of the gid table */
static void
create_group_table()
{
	struct group *tmp;
	int count, index;

	for (count = 0; getgrent(); count++) /* Do nothing */;

	gtable = xmalloc(sizeof(struct group) * (count + 1));
	memset(gtable, 0, sizeof(struct group) * (count + 1));
	setgrent();

	for (index = 0; (tmp = getgrent()) && index < count; index++) {
		gtable[index].gr_gid = tmp->gr_gid;
		gtable[index].gr_name = xstrdup(tmp->gr_name);
	}

	endgrent();
}

/* Caching version of getgrnam */
struct group *
xgetgrnam(const char *name)
{
	int i;

	if (!gtable)
		create_group_table();

	for (i = 0; gtable[i].gr_name; i++) {
		if (!strcmp(name, gtable[i].gr_name))
			return &(gtable[i]);
	}

	return NULL;
}

/* Caching version of getgrgid */
struct group *
xgetgrgid(gid_t gid)
{
	int i;

	if (!gtable)
		create_group_table();

	for (i = 0; gtable[i].gr_name; i++) {
		if (gtable[i].gr_gid == gid)
			return &(gtable[i]);
	}

	return NULL;
}

/* For user caching */
static struct passwd *ptable = NULL;

/* Initial setup of the passwd table */
static void
create_passwd_table()
{
	struct passwd *tmp;
	int count, index;

	for (count = 0; getpwent(); count++) /* Do nothing */;

	ptable = xmalloc(sizeof(struct passwd) * (count + 1));
	memset(ptable, 0, sizeof(struct passwd) * (count + 1));
	setpwent();

	for (index = 0; (tmp = getpwent()) && index < count; index++) {
		ptable[index].pw_uid = tmp->pw_uid;
		ptable[index].pw_name = xstrdup(tmp->pw_name);
	}

	endpwent();
}

/* Caching version of getpwnam */
struct passwd *
xgetpwnam(const char *name)
{
	int i;

	if (!ptable)
		create_passwd_table();

	for (i = 0; ptable[i].pw_name; i++) {
		if (!strcmp(name, ptable[i].pw_name))
			return &(ptable[i]);
	}

	return NULL;
}

/* Caching version of getpwuid */
struct passwd *
xgetpwuid(uid_t uid)
{
	int i;

	if (!ptable)
		create_passwd_table();

	for (i = 0; ptable[i].pw_name; i++) {
		if (ptable[i].pw_uid == uid)
			return &(ptable[i]);
	}

	return NULL;
}
