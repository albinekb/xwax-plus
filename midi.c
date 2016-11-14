/*
 * Copyright (C) 2016 Mark Hills <mark@xwax.org>
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

#include <stdio.h>

#include "midi.h"

/*
 * Print error code from ALSA
 */

static void alsa_error(const char *msg, int r)
{
    fprintf(stderr, "ALSA %s: %s\n", msg, snd_strerror(r));
}

int midi_open(struct midi *m, const char *name)
{
    int r;

    r = snd_rawmidi_open(&m->in, &m->out, name, SND_RAWMIDI_NONBLOCK);
    if (r < 0) {
        alsa_error("rawmidi_open", r);
        return -1;
    }

    return 0;
}

void midi_close(struct midi *m)
{
    if (snd_rawmidi_close(m->in) < 0)
        abort();
    if (snd_rawmidi_close(m->out) < 0)
        abort();
}

/*
 * Get the poll descriptors for reading on this MIDI device
 *
 * Pre: len is maximum size of array pe
 * Return: -1 if len is not large enough, otherwise n on success
 * Post: on success, pe is filled with n entries
 */

ssize_t midi_pollfds(struct midi *m, struct pollfd *pe, size_t len)
{
    int r;

    if (snd_rawmidi_poll_descriptors_count(m->in) > len)
        return -1;

    r = snd_rawmidi_poll_descriptors(m->in, pe, len);
    assert(r >= 0);

    return r;
}

/*
 * Read raw bytes of input
 *
 * Pre: len is maximum size of buffer
 * Return: -1 on error, otherwise n on success
 * Post: on success, buf is filled with n bytes of data
 */

ssize_t midi_read(struct midi *m, void *buf, size_t len)
{
    int r;

    r = snd_rawmidi_read(m->in, buf, len);
    if (r < 0) {
        if (r == -EAGAIN)
            return 0;
        alsa_error("rawmidi_read", r);
        return -1;
    }

    return r;
}

ssize_t midi_write(struct midi *m, const void *buf, size_t len)
{
    int r;

    r = snd_rawmidi_write(m->out, buf, len);
    if (r < 0) {
        if (r == -EAGAIN)
            return 0;
        alsa_error("rawmidi_write", r);
        return -1;
    }

    return r;
}
