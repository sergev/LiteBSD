/*
 * Copyright (c) 2016 Serge Vakulenko
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"

#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "opkg_download.h"
#include "opkg_message.h"

/*
 * Download using simple http request.
 */
int opkg_download_backend(const char *url, const char *dest,
                          curl_progress_func cb, void *data, int use_cache)
{
    char buf[8192], *bufp, *req = NULL;
    struct sockaddr_in addr;
    struct hostent *he;
    FILE *write_to;
    const char *s, *p, *filename, *hostname;
    int bytes, c, d;
    int sock = -1, file = -1, ret = -1;

    s = url + strlen("http://");
    p = strchr(s, '/');
    if (! p) {
        opkg_msg(ERROR, "Failed to download %s, empty filename.\n", url);
        return -1;
    }
    filename = p + 1;

    strncpy(buf, s, p-s);
    buf[p-s] = 0;
    hostname = buf;

    req = (char *)malloc(sizeof("GET ") + strlen(filename) + 3);
    if (!req) {
        opkg_msg(ERROR, "Failed to download %s, no memory.\n", url);
        return -1;
    }
    sprintf(req, "GET /%s\n", filename);

    he = gethostbyname(hostname);
    if (!he) {
        opkg_msg(ERROR, "Failed to download %s, hostname not found.\n", url);
        goto die;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        opkg_msg(ERROR, "Failed to download %s, cannot create socket.\n", url);
        goto die;
    }

    memset(&addr, 0, sizeof addr);
    addr.sin_family = he->h_addrtype;
    addr.sin_port = htons(80);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    if (connect(sock, (struct sockaddr *)&addr, sizeof addr) < 0) {
        opkg_msg(ERROR, "Failed to download %s, cannot connect.\n", url);
        goto die;
    }
    printf("Connected to %s.\n", hostname);

    printf("Retrieving using: %s", req);
    for (bufp = req, c = strlen(bufp); c > 0; c -= d, bufp += d) {
        d = write(sock, bufp, c);
        if (d <= 0)
            break;
    }
    if (d < 0) {
        opkg_msg(ERROR, "Failed to download %s, cannot send command.\n", url);
        goto die;
    }

    file = open(dest, O_CREAT|O_WRONLY|O_TRUNC, 0666);
    if (!file) {
        opkg_msg(ERROR, "Failed to download %s, cannot create file %s\n", url, dest);
        goto die;
    }

    bytes = 0;
    while ((c = read(sock, buf, sizeof (buf))) > 0) {
        bytes += c;
        for (bufp = buf; c > 0; c -= d, bufp += d) {
            d = write(file, bufp, c);
            if (d <= 0)
                break;
        }
    }
    if (d < 0) {
        opkg_msg(ERROR, "Failed to download %s, cannot receive data.\n", url);
        perror("failed to receive correctly");
        goto die;
    }
    //printf("Success, closing connection.\n");
    ret = 0;
die:
    if (sock >= 0)
        close(sock);
    if (file >= 0)
        close(file);
    if (req)
        free(req);
    return ret;
}

void opkg_download_cleanup(void)
{
    /* Nothing to do. */
}
