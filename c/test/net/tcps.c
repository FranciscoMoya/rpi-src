#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

static int tcp_master_socket(const char* service);

int main() {
    int master = tcp_master_socket("9999");
    for(;;) {
	int fd = accept(master, (struct sockaddr*) NULL, NULL);
	assert (fd >= 0);
	for(;;) {
	    char buf[1024];
	    int n = read(fd, buf, sizeof(buf));
	    assert(n >= 0);
	    if (n == 0) break;
	    buf[n]='\0';
	    printf("%s", buf);
	}
	close(fd);
    }
    close(master);
    return 0;
}

static struct sockaddr_in ip_address(const char* host, 
				     const char* service, 
				     const char* proto);

static int tcp_master_socket(const char* service)
{
    struct sockaddr_in sin = ip_address("0.0.0.0", service, "tcp");
    struct protoent* pe = getprotobyname("tcp");
    assert(pe != NULL);

    int fd = socket(PF_INET, SOCK_STREAM, pe->p_proto);
    assert (fd >= 0);
    assert(bind(fd, (struct sockaddr*)&sin, sizeof(sin)) >= 0);
    assert(listen(fd, 10) >= 0);
    return fd;
}


static struct sockaddr_in ip_address(const char* host, 
				     const char* service, 
				     const char* proto)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    struct hostent* he = gethostbyname(host);
    if (he != NULL)
	memcpy(&sin.sin_addr, he->h_addr_list[0], he->h_length);
    else
	sin.sin_addr.s_addr = inet_addr(host);
    struct servent* se = getservbyname(service, proto);
    sin.sin_port = (se != NULL? se->s_port : htons(atoi(service)));
    return sin;
}
