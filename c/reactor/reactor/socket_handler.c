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

static int master_socket(const char* service);
static int client_socket(const char* host, const char* service);
static void acceptor_free_members(acceptor* this);
static void connector_free_members(connector* this);
static void event_handler_close_and_free(event_handler* ev);

acceptor* acceptor_new(const char* service,
		       event_handler_function slave)
{
    acceptor* h = malloc(sizeof(acceptor));
    acceptor_init (h, service, slave);
    h->parent.destroy_self = (event_handler_function) free;
    return h;
}

static void acceptor_master_handle(acceptor* this);

void acceptor_init(acceptor* h,
		   const char* service,
		   event_handler_function slave)
{
    int fd = master_socket(service);
    event_handler_init (&h->parent, fd,
			(event_handler_function)acceptor_master_handle);
    h->slave = slave;
    h->destroy_parent_members = h->parent.destroy_members;
    h->parent.destroy_members = (event_handler_function) acceptor_free_members;    
}


void acceptor_destroy(acceptor* h)
{
    event_handler_destroy(&h->parent);
}


void event_handler_send(event_handler* h, const void* buf, size_t size)
{
    if (0 > write(h->fd, buf, size))
	Throw Exception(errno, "Imposible enviar");
}


size_t event_handler_recv(event_handler* h, void* buf, size_t size)
{
    int ret = read(h->fd, buf, size);
    if (ret <= 0)
	Throw Exception(errno, "Imposible leer");
    return ret;
}


static void acceptor_master_handle(acceptor* this)
{
    event_handler* ev = &this->parent;
    int fd = accept(ev->fd, (struct sockaddr*) NULL, NULL);
    if (fd < 0) return;
    event_handler* slave = event_handler_new(fd, this->slave);
    slave->destroy_self = event_handler_close_and_free;
    reactor_add(ev->r, slave);
}


connector* connector_new(const char* host,
			 const char* service,
			 event_handler_function handler)
{
    connector* h = malloc(sizeof(connector));
    connector_init (h, host, service, handler);
    h->parent.destroy_self = (event_handler_function) free;
    return h;
}


void connector_init(connector* h,
		    const char* host,
		    const char* service,
		    event_handler_function handler)
{
    int fd = client_socket(host, service);
    event_handler_init (&h->parent, fd, handler);
    h->destroy_parent_members = h->parent.destroy_members;
    h->parent.destroy_members = (event_handler_function) connector_free_members;
}


void connector_destroy(connector* h)
{
    event_handler_destroy(&h->parent);
}


void connector_send(connector* h, const void* buf, size_t size)
{
    event_handler_send(&h->parent, buf, size);
}


size_t connector_recv(connector* h, void* buf, size_t size)
{
    return event_handler_recv(&h->parent, buf, size);
}


static int master_socket(const char* service)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);

    struct servent* se = getservbyname(service, "tcp");
    sin.sin_port =  (se != NULL? se->s_port : htons(atoi(service)));

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


static int client_socket(const char* host, const char* service)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    struct hostent* he = gethostbyname(host);
    if (he != NULL)
	memcpy(&sin.sin_addr, he->h_addr_list[0], he->h_length);
    else
	sin.sin_addr.s_addr = inet_addr(host);

    struct servent* se = getservbyname(service, "tcp");
    sin.sin_port =  (se != NULL? se->s_port : htons(atoi(service)));

    struct protoent* pe = getprotobyname("tcp");
    Assert(pe != NULL);

    int fd = socket(PF_INET, SOCK_STREAM, pe->p_proto);
    Assert (fd >= 0);
    
    if (0 > connect(fd, (struct sockaddr*)&sin, sizeof(sin)))
	Throw Exception(errno, "Error en connect");

    return fd;
}


static void acceptor_free_members(acceptor* this)
{
    this->destroy_parent_members(&this->parent);
    close(this->parent.fd);
}


static void connector_free_members(connector* this)
{
    this->destroy_parent_members(&this->parent);
    close(this->parent.fd);
}


static void event_handler_close_and_free(event_handler* ev)
{
    close(ev->fd);
    free(ev);
}
