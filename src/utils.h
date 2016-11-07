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

/**
 * @file utils.h
 * @author David Härdeman
 * @brief File declaring a set of functions used in metastore
 * 
 * The functions declared here are used by other functions in metastore.
 */

#ifndef UTILS_H
#define UTILS_H

/* For uint64_t */
#include <stdint.h>
/* For ssize_t */
#include <unistd.h>
/* For FILE */
#include <stdio.h>
/* For struct passwd */
#include <pwd.h>
/* For struct group */
#include <grp.h>

/**
 * Adjusts the verbosity level for msg()
 * @param adj Number defining the verbosity level (see MSG_* macros)
 */
void adjust_verbosity(int adj);

/* Verbosity levels using stdout */
#define MSG_NORMAL    0
#define MSG_DEBUG     1
#define MSG_QUIET    -1
/* Verbosity levels using stderr */
#define MSG_ERROR    -2
#define MSG_CRITICAL -3

/**
 * Prints messages to console according to the current verbosity
 * @param level Verbosity level that shall be usd for this message
 * @param fmt   String format printf-style
 * @return Number of printed characters
 */
int msg(int level, const char *fmt, ...);

/**
 * Malloc which either succeeds or exits
 * @param size Size of the memory area that shall be allocated
 * @return Pointer to the allocated memory
 */
void *xmalloc(size_t size);

/**
 * Strdup which either succeeds or exits
 * @param s String that shall be duplicated
 * @return Pointer to the duplicated string
 */
char *xstrdup(const char *s);

/**
 * Human-readable printout of binary data
 * @param s   Byte array that shall be printed
 * @param len Number of bytes that shall be printed 
 */
void binary_print(const char *s, ssize_t len);

/**
 * Writes data to a file or exits on failure
 * @param ptr    Pointer to generic data that shall be written
 * @param size   Size of data that shall be written
 * @param stream File that shall be written to
 */
void xfwrite(const void *ptr, size_t size, FILE *stream);

/**
 * Writes an int to a file, using len bytes, in bigendian order
 * @param value Integer that shall be written
 * @param len   Bytes that shall be written
 * @param to    File that shall be written to
 */
void write_int(uint64_t value, size_t len, FILE *to);

/**
 * Writes a binary string to a file
 * @param string String that shall be written
 * @param len    Bytes that shall be written
 * @param to     File that shall be written to
 */
void write_binary_string(const char *string, size_t len, FILE *to);

/**
 * Writes a normal C string to a file
 * @param string String that shall be written
 * @param to     File that shall be written to
 */
void write_string(const char *string, FILE *to);

/**
 * Reads an int from a string, using len bytes, in bigendian order
 * @param from Pointer to string representation of the file
 * @param len  Bytes that shall be read for this integer
 * @param max  Pointer to the end of the buffer
 * @return Integer read from the file
 */
uint64_t read_int(char **from, size_t len, const char *max);

/**
 * Reads a binary string from a file
 * @param from Pointer to string representation of the file
 * @param len  Bytes that shall be read for this string
 * @param max  Pointer to the end of the buffer
 * @return String read from the file
 */
char *read_binary_string(char **from, size_t len, const char *max);

/**
 * Reads a normal C string from a file
 * @param from Pointer to string representation of the file
 * @param max  Pointer to the end of the buffer
 * @return Null-teminated string read from the file
 */
char *read_string(char **from, const char *max);

/**
 * Caching version of getgrnam
 * @param name Group name that shall be searched
 * @return Pointer to struct group if found, NULL otherwise
 */
struct group *xgetgrnam(const char *name);

/**
 * Caching version of getgrgid
 * @param name Group id that shall be searched
 * @return Pointer to struct group if found, NULL otherwise
 */
struct group *xgetgrgid(gid_t gid);

/**
 * Caching version of getpwnam
 * @param name User name that shall be searched
 * @return Pointer to struct passwd if found, NULL otherwise
 */
struct passwd *xgetpwnam(const char *name);

/**
 * Caching version of getpwuid
 * @param name User id that shall be searched
 * @return Pointer to struct passwd if found, NULL otherwise
 */
struct passwd *xgetpwuid(uid_t uid);

#endif /* UTILS_H */
