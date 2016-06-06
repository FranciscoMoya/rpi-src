#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

static int udp_client_socket(const char* host, const char* service);

int main() {
    int fd = udp_client_socket("localhost", "9999");
    for(;;) {
	char buf[1024];
	fgets(buf, sizeof(buf), stdin);
	int n = write(fd, buf, strlen(buf));
	assert(n >= 0);
	if (n == 0) break;
    }
    close(fd);
    return 0;
}

static struct sockaddr_in ip_address(const char* host, 
				     const char* service, 
				     const char* proto);

static int udp_client_socket(const char* host, const char* service)
{
    struct sockaddr_in sin = ip_address(host, service, "udp");
    struct protoent* pe = getprotobyname("udp");
    assert(pe != NULL);
    int fd = socket(PF_INET, SOCK_DGRAM, pe->p_proto);
    assert (fd >= 0);
    assert (connect(fd, (struct sockaddr*)&sin, sizeof(sin)) >= 0);
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
