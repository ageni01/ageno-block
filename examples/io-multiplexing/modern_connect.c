#define _POSIX_C_SOURCE 200809L
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
/* Blocking teaching example: successful return transfers fd ownership to caller. */
static int connect_host(const char *host, const char *service) {
    struct addrinfo hints = {0}, *addresses = NULL;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int rc = getaddrinfo(host, service, &hints, &addresses);
    if (rc != 0) { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc)); return -1; }
    int fd = -1, saved = ECONNREFUSED;
    for (struct addrinfo *p = addresses; p; p = p->ai_next) {
        fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd < 0) { saved = errno; continue; }
        if (connect(fd, p->ai_addr, p->ai_addrlen) == 0) break;
        saved = errno; close(fd); fd = -1;
    }
    freeaddrinfo(addresses);
    if (fd < 0) errno = saved;
    return fd;
}
int main(int argc, char **argv) {
    int fd = connect_host(argc > 1 ? argv[1] : "localhost", argc > 2 ? argv[2] : "9090");
    if (fd < 0) { perror("connect"); return 1; }
    struct sockaddr_storage local, remote;
    socklen_t lsize = sizeof local, rsize = sizeof remote;
    char lh[1025], ls[32], rh[1025], rs[32];
    if (getsockname(fd, (struct sockaddr *)&local, &lsize) < 0 ||
        getpeername(fd, (struct sockaddr *)&remote, &rsize) < 0) { perror("endpoint"); close(fd); return 1; }
    int a = getnameinfo((struct sockaddr *)&local, lsize, lh, sizeof lh, ls, sizeof ls, NI_NUMERICHOST | NI_NUMERICSERV);
    int b = getnameinfo((struct sockaddr *)&remote, rsize, rh, sizeof rh, rs, sizeof rs, NI_NUMERICHOST | NI_NUMERICSERV);
    if (a || b) { fprintf(stderr, "getnameinfo: %s\n", gai_strerror(a ? a : b)); close(fd); return 1; }
    printf("local=[%s]:%s remote=[%s]:%s\n", lh, ls, rh, rs);
    close(fd);
}
