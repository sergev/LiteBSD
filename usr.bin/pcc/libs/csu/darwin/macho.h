/*	$Id: macho.h,v 1.2 2011/05/27 07:23:49 gmcgarry Exp $	*/
/*-
 * Copyright (c) 2008 Gregory McGarry <g.mcgarry@ieee.org>
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

#include <stdint.h>

struct mach_header {
        uint32_t        magic;
        uint32_t        cputype;
        uint32_t        cpusubtype;
        uint32_t        filetype;
        uint32_t        ncmds;
        uint32_t        sizeofcmds;
        uint32_t        flags;
};

#define LC_SEGMENT	0x01

struct segment_command_32 {
        uint32_t        cmd;
        uint32_t        cmdsize;
        char            segname[16];
        uint32_t        vmaddr;
        uint32_t        vmsize;
        uint32_t        fileoff;
        uint32_t        filesize;
        uint32_t        maxprot;
        uint32_t        initprot;
        uint32_t        nsects;
        uint32_t        flags;
};

struct segment_command_64 {
        uint32_t        cmd;
        uint32_t        cmdsize;
        char            segname[16];
        uint64_t        vmaddr;
        uint64_t        vmsize;
        uint64_t        fileoff;
        uint64_t        filesize;
        uint32_t        maxprot;
        uint32_t        initprot;
        uint32_t        nsects;
        uint32_t        flags;
};

struct section_32 {
        char            sectname[16];
        char            segname[16];
        uint32_t        addr;
        uint32_t        size;
        uint32_t        offset;
        uint32_t        align;
        uint32_t        reloff;
        uint32_t        nreloc;
        uint32_t        flags;
        uint32_t        reserved1;
        uint32_t        reserved2;
};

struct section_64 {
        char            sectname[16];
        char            segname[16];
        uint64_t        addr;
        uint64_t        size;
        uint32_t        offset;
        uint32_t        align;
        uint32_t        reloff;
        uint32_t        nreloc;
        uint32_t        flags;
        uint32_t        reserved1;
        uint32_t        reserved2;
};
