#include <reactor/socket_handler.h>
#include <reactor/reactor.h>
#include <reactor/exception.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define ACCEPTOR_BACKLOG 5


endpoint* endpoint_new(int sock, 
		       event_handler_function handler)
{
    endpoint* h = malloc(sizeof(endpoint));
    endpoint_init (h, sock, handler);
    h->parent.destroy_self = (event_handler_function) free;
    return h;
}


static void endpoint_free_members(endpoint* this);

void endpoint_init(endpoint* this, 
		   int sock, 
		   event_handler_function handler)
{
    event_handler_init (&this->parent, sock, handler);
    this->destroy_parent_members = this->parent.destroy_members;
    this->parent.destroy_members = (event_handler_function) endpoint_free_members;    
}


void endpoint_destroy(endpoint* this)
{
    event_handler_destroy(&this->parent);
}


void endpoint_send(endpoint* this, const void* buf, size_t size)
{
    event_handler* h = (event_handler*)this;
    if (0 > write(h->fd, buf, size))
	Throw Exception(errno, "Imposible enviar");
}


size_t endpoint_recv(endpoint* this, void* buf, size_t size)
{
    event_handler* h = (event_handler*)this;
    int ret = read(h->fd, buf, size);
    if (ret <= 0)
	Throw Exception(errno, "Imposible leer");
    return ret;
}


acceptor* acceptor_new(const char* service,
		       event_handler_function slave)
{
    acceptor* h = malloc(sizeof(acceptor));
    event_handler* ev = (event_handler*)h;
    acceptor_init (h, service, slave);
    ev->destroy_self = (event_handler_function) free;
    return h;
}


static int tcp_master_socket(const char* service);
static void acceptor_master_handle(acceptor* this);

void acceptor_init(acceptor* h,
		   const char* service,
		   event_handler_function slave)
{
    int fd = tcp_master_socket(service);
    endpoint_init (&h->parent, fd,
		   (event_handler_function)acceptor_master_handle);
    h->slave = slave;
}


void acceptor_destroy(acceptor* h)
{
    event_handler_destroy((event_handler*)h);
}


static int tcp_client_socket(const char* host, const char* service);

static void acceptor_master_handle(acceptor* this)
{
    event_handler* ev = (event_handler*)this;
    int fd = accept(ev->fd, (struct sockaddr*) NULL, NULL);
    if (fd < 0) return;
    reactor_add(ev->r, (event_handler*)endpoint_new(fd, this->slave));
}


static int tcp_client_socket(const char* host, const char* service);

endpoint* connector_new(const char* host,
			const char* service,
			event_handler_function handler)
{
    return endpoint_new(tcp_client_socket(host, service),
			handler);
}


void connector_init(endpoint* this,
		    const char* host,
		    const char* service,
		    event_handler_function handler)
{
    endpoint_init(this, 
		  tcp_client_socket(host, service),
		  handler);
}


static int udp_server_socket(const char* service);

endpoint* udp_endpoint_new(const char* service,
			   event_handler_function handler)
{
    return endpoint_new(udp_server_socket(service),
			handler);
}


void udp_endpoint_init(endpoint* this,
		       const char* service,
		       event_handler_function handler)
{
    endpoint_init(this, 
		  udp_server_socket(service),
		  handler);
}


static int udp_client_socket(const char* host, const char* service);

endpoint* udp_connector_new(const char* host,
			    const char* service,
			    event_handler_function handler)
{
    return endpoint_new(udp_client_socket(host, service),
			handler);
}


void udp_connector_init(endpoint* this,
			const char* host,
			const char* service,
			event_handler_function handler)
{
    endpoint_init(this, 
		  udp_client_socket(host, service),
		  handler);
}


static void endpoint_free_members(endpoint* this)
{
    this->destroy_parent_members(&this->parent);
    close(this->parent.fd);
}


static struct sockaddr_in tcp_address(const char* host, const char* service);

static int tcp_master_socket(const char* service)
{
    struct sockaddr_in sin = tcp_address("0.0.0.0", service);

    struct protoent* pe = getprotobyname("tcp");
    Assert(pe != NULL);

    int fd = socket(PF_INET, SOCK_STREAM, pe->p_proto);
    Assert (fd >= 0);
    
    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (0 > bind(fd, (struct sockaddr*)&sin, sizeof(sin)))
	Throw Exception(errno, "Error en bind");

    if (0 > listen(fd, ACCEPTOR_BACKLOG))
	Throw Exception(errno, "Error en listen");

    return fd;
}


static int tcp_client_socket(const char* host, const char* service)
{
    struct sockaddr_in sin = tcp_address(host, service);

    struct protoent* pe = getprotobyname("tcp");
    Assert(pe != NULL);

    int fd = socket(PF_INET, SOCK_STREAM, pe->p_proto);
    Assert (fd >= 0);
    
    if (0 > connect(fd, (struct sockaddr*)&sin, sizeof(sin)))
	Throw Exception(errno, "Error en connect");

    return fd;
}


static struct sockaddr_in udp_address(const char* host, const char* service);

static int udp_server_socket(const char* service)
{
    struct sockaddr_in sin = udp_address("0.0.0.0", service);

    struct protoent* pe = getprotobyname("udp");
    Assert(pe != NULL);

    int fd = socket(PF_INET, SOCK_DGRAM, pe->p_proto);
    Assert (fd >= 0);
    
    if (0 > bind(fd, (struct sockaddr*)&sin, sizeof(sin)))
	Throw Exception(errno, "Error en bind");

    return fd;
}


static int udp_client_socket(const char* host, const char* service)
{
    struct sockaddr_in sin = udp_address(host, service);

    struct protoent* pe = getprotobyname("udp");
    Assert(pe != NULL);

    int fd = socket(PF_INET, SOCK_DGRAM, pe->p_proto);
    Assert (fd >= 0);
    
    if (0 > connect(fd, (struct sockaddr*)&sin, sizeof(sin)))
	Throw Exception(errno, "Error en connect");

    return fd;
}


static struct sockaddr_in ip_address(const char* host, 
				     const char* service, 
				     const char* proto);

static struct sockaddr_in tcp_address(const char* host, const char* service)
{
    return ip_address(host, service, "tcp");
}


static struct sockaddr_in udp_address(const char* host, const char* service)
{
    return ip_address(host, service, "udp");
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
