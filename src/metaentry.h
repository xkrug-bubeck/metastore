/*
 * Various functions to work with meta entries.
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
 * @file metaentry.h
 * @author David Härdeman
 * @brief File declaring the metaentry and metahash structures for storing the file/dir metadata
 * 
 * The metaentry object is part of a linked list to store
 * the owner, group, rights and times of a file or folder.
 */

#ifndef METAENTRY_H
#define METAENTRY_H

#include <stdbool.h>

#include "settings.h"

/**
 * @brief Data structure to hold all metadata for a file/dir
 * 
 * Structure for storing the file properties and permissions.
 * It is a direct member of either the metahash bucket or is in 
 * the the "next" list of such a member.
 */
struct metaentry {
	struct metaentry *next; /**< Pointer to the next list member of this metahash bucket */
	struct metaentry *list; /**< Pointer to an additional lists of entries */

	char    *path;		/**< Pointer to the item path */
	unsigned pathlen;	/**< Length of the path name */

	char    *owner;		/**< Pointer to the name of the owner */
	char    *group;		/**< Pointer to the group name */
	mode_t   mode;		/**< Access mode storage */
	time_t   mtime;		/**< Modification time as struct time_t */
	long     mtimensec;	/**< Modification time as long integer */

	unsigned xattrs;	/**< The numbers of extended attributes stored for this item */
	char   **xattr_names;	/**< Pointer to a list of xattr names */
	ssize_t *xattr_lvalues; /**< Number of xattr_values items */
	char   **xattr_values;	/**< Pointer to a list of xattr values */
};

/**
 * Size of metahash bucket array size
 */
#define HASH_INDEXES 1024

/**
 * @brief Data structure to hold a number of metadata entries
 * 
 * The metahash structure stores the meta entries into a number
 * of hash buckets.
 */
struct metahash {
	struct metaentry *bucket[HASH_INDEXES];	/**< Array for storing the metaentry lists according to their hashes */
	unsigned count;				/**< Number of meta hashes used */
};

/**
 * Create a metaentry for the file/dir/etc at path
 * @param path The file system path of the item the ethry shall be created for
 * @return Pointer to the created metaentry struct
 */
struct metaentry *mentry_create(const char *path);

/**
 * Recurses opath and adds metadata entries to the metaentry list
 * @param opath Path that shall be recursed
 * @param mhash Pointer to the storage of the metahash list
 * @param st    Pointer to application settings struct
 */
void mentries_recurse_path(const char *opath, struct metahash **mhash,
                           msettings *st);

/**
 * Stores a metaentry list to a file
 * @param mhash Pointer to the metahash struct
 * @param path  Path of the file that shall be written
 */
void mentries_tofile(const struct metahash *mhash, const char *path);

/**
 * Creates a metaentry list from a file
 * @param mhash Pointer to the storage of the metahash list
 * @param path  Path of the file that shall be read
 */
void mentries_fromfile(struct metahash **mhash, const char *path);

/**
 * Searches haystack for an xattr matching xattr number n in needle
 * @param haystack The metaantry that shall be searched
 * @param needle   The metaentry that contains the xattr that shall be searched
 * @param n        Index of the xattrs that shall be searched
 * @return Index of the macthing element or -1 if element is not found or does not match
 */
int mentry_find_xattr(struct metaentry *haystack,
                      struct metaentry *needle,
                      unsigned n);

#define DIFF_NONE  0x00
#define DIFF_OWNER 0x01
#define DIFF_GROUP 0x02
#define DIFF_MODE  0x04
#define DIFF_TYPE  0x08
#define DIFF_MTIME 0x10
#define DIFF_XATTR 0x20
#define DIFF_ADDED 0x40
#define DIFF_DELE  0x80

/**
 * Compares two metaentries and returns an int with a bitmask of differences
 * @param left  Pointer to a metaentry struct for comparison
 * @param right Pointer to a metaentry struct for comparison
 * @param st    Pointer to application settings struct
 * @return Bitmask of differences (see DIFF_* macros)
 */
int mentry_compare(struct metaentry *left,
                   struct metaentry *right,
                   msettings *st);

/**
 * Compares lists of real and stored metadata and calls pfunc for each
 * @param mhashreal   Pointer to metahash structure with current data
 * @param mhashstored Pointer to metahash structure with stored data
 * @param pfunc       Pointer to function that shall be executed if an entry does not match
 * @param st          Pointer to application settings struct
 */
void mentries_compare(struct metahash *mhashreal,
                      struct metahash *mhashstored,
                      void (*pfunc)(struct metaentry *real,
                                    struct metaentry *stored,
                                    int cmp),
                      msettings *st);
/**
 * Dump the contents of the metahash bucket in readable way on stdout
 * @param mhash Pointer to the metahash struct
 */
void mentries_dump(struct metahash *mhash);

#endif /* METAENTRY_H */
