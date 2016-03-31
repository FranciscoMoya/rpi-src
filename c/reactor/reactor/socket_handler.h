#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

/* Clases para las comunicaciones TCP utilizando el patrón
   acceptor/connector.
 */

#include <reactor/event_handler.h>
#include <netinet/in.h>

typedef struct slave_handler_ slave_handler;
typedef struct acceptor_ acceptor;
typedef struct connector_ connector;

struct acceptor_ {
    event_handler parent;
    event_handler_function destroy_parent_members;
    event_handler_function slave;
};

struct connector_ {
    event_handler parent;
    event_handler_function destroy_parent_members;
};

acceptor* acceptor_new(const char* service,
		       event_handler_function slave);
void acceptor_init(acceptor* h,
		   const char* service,
		   event_handler_function slave);
void acceptor_destroy(acceptor* h);

void event_handler_send(event_handler* h, const void* buf, size_t size);
size_t event_handler_recv(event_handler* h, void* buf, size_t size);

connector* connector_new(const char* host,
			 const char* service,
			 event_handler_function handler);
void connector_init(connector* this,
		    const char* host,
		    const char* service,
		    event_handler_function handler);
void connector_destroy(connector* this);

void connector_send(connector* h, const void* buf, size_t size);
size_t connector_recv(connector* h, void* buf, size_t size);

#endif
