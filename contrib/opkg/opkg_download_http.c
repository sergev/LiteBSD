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
#include <fcntl.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "opkg_download.h"
#include "opkg_message.h"

#define HTTP_GET            "GET %s HTTP/1.0\r\nHost: %s\r\n\r\n"

/*
 * Read one line from file descriptor.
 */
static int get_line(FILE *fd, char *str, int sz)
{
    char *beg = str;
    int c;

    /* Check for EOF. */
    c = getc(fd);
    if (c < 0)
        return -1;
    ungetc(c, fd);

    while (--sz>0 && (c = getc(fd)) >= 0 && c != '\n')
        *str++ = c;

    /* Trim trailing CR. */
    if (str > beg && str[-1] == '\r')
        --str;
    *str = 0;
    return 0;
}

/*
 * Download using simple http request.
 */
int opkg_download_backend(const char *url, const char *dest,
                          curl_progress_func cb, void *data, int use_cache)
{
    char buf[4096], *bufp, *req = NULL;
    struct sockaddr_in addr;
    struct hostent *he;
    FILE *fd = 0;
    const char *hostp, *filepath, *hostname;
    int bytes, c, d, status;
    int sock = -1, file = -1, ret = -1;

    hostp = url + strlen("http://");
    filepath = strchr(hostp, '/');
    if (! filepath) {
        opkg_msg(ERROR, "Failed to download %s, empty filename.\n", url);
        return -1;
    }

    strncpy(buf, hostp, filepath - hostp);
    buf[filepath - hostp] = 0;
    hostname = buf;

    req = alloca(sizeof(HTTP_GET) + strlen(url));
    if (!req) {
        opkg_msg(ERROR, "Failed to download %s, no memory.\n", url);
        return -1;
    }
    sprintf(req, HTTP_GET, filepath, hostname);

    he = gethostbyname(hostname);
    if (!he) {
        opkg_msg(ERROR, "Failed, hostname %s not found.\n", hostname);
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
        opkg_msg(ERROR, "%s: Cannot connect.\n", hostname);
        goto die;
    }
    opkg_msg(DEBUG, "Connected to %s.\n", hostname);

    opkg_msg(DEBUG, "Retrieving using: %s", req);
    for (bufp = req, c = strlen(bufp); c > 0; c -= d, bufp += d) {
        d = write(sock, bufp, c);
        if (d <= 0)
            break;
    }
    if (d < 0) {
        opkg_msg(ERROR, "%s: Cannot send command.\n", hostname);
        goto die;
    }

    file = open(dest, O_CREAT|O_WRONLY|O_TRUNC, 0666);
    if (!file) {
        opkg_msg(ERROR, "Cannot create file %s\n", dest);
        goto die;
    }

    /* Get response status line. */
    fd = fdopen(sock, "r");
    if (get_line(fd, buf, sizeof buf) < 0) {
fail:   opkg_msg(ERROR, "Cannot receive data from %s.\n", hostname);
        unlink(dest);
        goto die;
    }
    opkg_msg(DEBUG2, "--- %s\n", buf);
    if (strncmp(buf, "HTTP/1.0 ", 9) != 0 &&
        strncmp(buf, "HTTP/1.1 ", 9) != 0) {
        goto fail;
    }
    status = atoi(buf + 9);

    /* Skip response header. */
    while (get_line(fd, buf, sizeof buf) >= 0) {
        opkg_msg(DEBUG2, "--- %s\n", buf);
        if (buf[0] == 0)
            break;
    }

    /* Get message body. */
    bytes = 0;
    while ((c = fread(buf, 1, sizeof buf, fd)) > 0) {
        bytes += c;
        for (bufp = buf; c > 0; c -= d, bufp += d) {
            d = write(file, bufp, c);
            if (d != c)
                goto fail;
        }
    }
    if (status != 200) {
        /* Bad response status code. */
        opkg_msg(ERROR, "%s: Bad HTTP response code %d.\n", hostname, status);
        unlink(dest);
        goto die;
    }
    opkg_msg(DEBUG, "Success, got %u bytes, closing connection.\n", bytes);
    ret = 0;
die:
    if (sock >= 0)
        close(sock);
    if (file >= 0)
        close(file);
    if (fd)
        fclose(fd);
    return ret;
}

void opkg_download_cleanup(void)
{
    /* Nothing to do. */
}
