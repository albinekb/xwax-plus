/*
 * Copyright (C) 2018 Mark Hills <mark@xwax.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License version 2 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2 along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#define _GNU_SOURCE /* strcasestr(), strdupa() */
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "index.h"

#define BLOCK 1024
#define MAX_WORDS 32
#define SEPARATOR ' '

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

/*
 * Initialise a record index
 */

void index_init(struct index *ls)
{
    ls->record = NULL;
    ls->size = 0;
    ls->entries = 0;
}

/*
 * Deallocate resources associated with this index
 *
 * The index does not allocate records itself, so it is not
 * responsible for deallocating them.
 */

void index_clear(struct index *ls)
{
    free(ls->record); /* may be NULL */
}

/*
 * Blank the index so it contains no entries
 *
 * We don't de-allocate memory, but this gives us an advantage where
 * index re-use is of similar size.
 */

void index_blank(struct index *ls)
{
    ls->entries = 0;
}

/*
 * Enlarge the storage space of the index to at least the target
 * size
 *
 * Return: 0 on success or -1 on memory allocation failure
 * Post: size of index is greater than or equal to target
 */

static int enlarge(struct index *ls, size_t target)
{
    size_t p;
    struct record **ln;

    if (target <= ls->size)
        return 0;

    p = target + BLOCK - 1; /* pre-allocate additional entries */

    ln = realloc(ls->record, sizeof(struct record*) * p);
    if (ln == NULL) {
        perror("realloc");
        return -1;
    }

    ls->record = ln;
    ls->size = p;
    return 0;
}

/*
 * Return: false if the caller did not call index_reserve(), otherwise
 * true
 */

static bool has_space(const struct index *i)
{
    return i->entries < i->size;
}

/*
 * Add a record to the index
 *
 * Pre: at least one entry is reserved
 * Post: lr is the record at the end of the index
 */

void index_add(struct index *ls, struct record *lr)
{
    assert(lr != NULL);
    assert(has_space(ls));

    ls->record[ls->entries++] = lr;
}

/*
 * Standard comparison function between two records
 */

static int record_cmp_artist(const struct record *a, const struct record *b)
{
    int r;

    r = strcasecmp(a->artist, b->artist);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    r = strcasecmp(a->title, b->title);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    return strcmp(a->pathname, b->pathname);
}


void record_set_played(struct record *re){

    re->status = RECORD_PLAYED;
}

/*
 * Standard comparison function between two records -> album
 */

static int record_cmp_album(const struct record *a, const struct record *b)
{
    int r;
    
    r = strcasecmp(a->album, b->album);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    r = strcasecmp(a->artist, b->artist);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    r = strcasecmp(a->title, b->title);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    return record_cmp_artist(a, b);
}

/*
 * Standard comparison function between two records -> genre
 */

static int record_cmp_genre(const struct record *a, const struct record *b)
{
    int r;
    
    r = strcasecmp(a->genre, b->genre);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    r = strcasecmp(a->album, b->album);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    r = strcasecmp(a->artist, b->artist);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    r = strcasecmp(a->title, b->title);
    if (r < 0)
        return -1;
    else if (r > 0)
        return 1;

    return record_cmp_album(a, b);
}

/*
 * Compare two records principally by BPM, fastest to slowest
 * followed by unknown
 */

static int record_cmp_bpm(const struct record *a, const struct record *b)
{
    if (a->bpm < b->bpm)
        return -1;

    if (a->bpm > b->bpm)
        return 1;

    return record_cmp_album(a, b);
}

/*
 * Check if a record matches the given string. This function is the
 * definitive code which defines what constitutes a 'match'.
 *
 * Return: true if this is a match, otherwise false
 */

static bool record_match_word(struct record *re, const char *match)
{
    /* Some records provide a dedicated string for matching against,
     * in the same locale as "match" */

    if (re->match) {
        if (strcasestr(re->match, match) != NULL)
            return true;
    } else {
            
        
    if (strcasestr(re->artist, match) != NULL)
        return true;
    if (strcasestr(re->title, match) != NULL)
        return true;
    if (strcasestr(re->album, match) != NULL)
        return true;
    if (strcasestr(re->genre, match) != NULL)
        return true;
}
    return false;
}

/*
 * Check for a match against the given search criteria
 *
 * Return: true if the given record matches, otherwise false
 */

bool record_match(struct record *re, const struct match *h)
{
    char *const *matches;

    matches = h->words;

    while (*matches != NULL) {
        if (!record_match_word(re, *matches))
            return false;
        matches++;
    }
    return true;
}

/*
 * Copy the source index
 *
 * Return: 0 on success or -1 on memory allocation failure
 * Post: on failure, dest is valid but incomplete
 */

int index_copy(const struct index *src, struct index *dest)
{
    int n;

    index_blank(dest);

    if (index_reserve(dest, src->entries) == -1)
        return -1;

    for (n = 0; n < src->entries; n++)
        index_add(dest, src->record[n]);

    return 0;
}

/*
 * Compile a search object from a given string
 *
 * Pre: search string is within length
 */

void match_compile(struct match *h, const char *d)
{
    char *buf;
    size_t n;

    assert(strlen(d) < sizeof h->buf);
    strcpy(h->buf, d);

    buf = h->buf;
    n = 0;
    for (;;) {
        char *s;

        if (n == ARRAY_SIZE(h->words) - 1) {
            fputs("Ignoring excessive words in match string.\n", stderr);
            break;
        }

        h->words[n] = buf;
        n++;

        s = strchr(buf, SEPARATOR);
        if (s == NULL)
            break;
        *s = '\0';
        buf = s + 1; /* skip separator */
    }
    h->words[n] = NULL; /* terminate list */
}

/*
 * Find entries from the source index which match
 *
 * Copy the subset of the source index which matches the given
 * string into the destination.
 *
 * Return: 0 on success, or -1 on memory allocation failure
 * Post: on failure, dest is valid but incomplete
 */

int index_match(struct index *src, struct index *dest,
                const struct match *match)
{
    int n;
    struct record *re;

    index_blank(dest);

    for (n = 0; n < src->entries; n++) {
        re = src->record[n];

        if (record_match(re, match)) {
            if (index_reserve(dest, 1) == -1)
                return -1;
            index_add(dest, re);
        }
    }

    return 0;
}

/*
 * Binary search of sorted index
 *
 * We implement our own binary search rather than using the bsearch()
 * from stdlib.h, because we need to know the position to insert to if
 * the item is not found.
 *
 * Pre: base is sorted
 * Return: position of match >= item
 * Post: on exact match, *found is true
 */

static size_t bin_search(struct record **base, size_t n,
                         struct record *item, int sort,
                         bool *found)
{
    int r;
    size_t mid;
    struct record *x;

    /* Return the first entry ordered after this one */

    if (n == 0) {
        *found = false;
        return 0;
    }

    mid = n / 2;
    x = base[mid];

    switch (sort) {
    case SORT_ARTIST:
        r = record_cmp_artist(item, x);
        break;
    case SORT_ALBUM:
        r = record_cmp_album(item, x);
        break;
    case SORT_BPM:
        r = record_cmp_bpm(item, x);
        break;
    case SORT_GENRE:
        r = record_cmp_genre(item, x);
        break;
    case SORT_PLAYLIST:
    default:
        abort();
    }

    if (r < 0)
        return bin_search(base, mid, item, sort, found);
    if (r > 0) {
        return mid + 1
            + bin_search(base + mid + 1, n - mid - 1, item, sort, found);
    }

    *found = true;
    return mid;
}

/*
 * Insert or re-use an entry in a sorted index
 *
 * Pre: index is sorted
 * Pre: at least one entry is reserved
 * Return: pointer to item, or existing entry (ie. not NULL)
 * Post: index is sorted and contains item or a matching item
 */

struct record* index_insert(struct index *ls, struct record *item,
                            int sort)
{
    bool found;
    size_t z;

    z = bin_search(ls->record, ls->entries, item, sort, &found);
    if (found)
        return ls->record[z];

    assert(has_space(ls));

    memmove(ls->record + z + 1, ls->record + z,
            sizeof(struct record*) * (ls->entries - z));
    ls->record[z] = item;
    ls->entries++;

    return item;
}

/*
 * Reserve space in the index for the addition of n new items
 *
 * This function exists separately to the insert and addition
 * functions because it carries the error case.
 *
 * Return: -1 if not enough memory, otherwise zero
 * Post: if zero is returned, index has at least n free slots
 */

int index_reserve(struct index *i, unsigned int n)
{
    return enlarge(i, i->entries + n);
}

/*
 * Find an identical entry, or the nearest match
 */

size_t index_find(struct index *ls, struct record *item, int sort)
{
    bool found;
    size_t z;

    z = bin_search(ls->record, ls->entries, item, sort, &found);
    return z;
}

/*
 * Debug the content of a index to standard error
 */

void index_debug(struct index *ls)
{
    int n;

    for (n = 0; n < ls->entries; n++)
        fprintf(stderr, "%d: %s\n", n, ls->record[n]->pathname);
}
