#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

/* Clases para las comunicaciones TCP utilizando el patrón
   acceptor/connector.
 */

#include <reactor/event_handler.h>
#include <netinet/in.h>

typedef struct endpoint_ endpoint;
struct endpoint_ {
    event_handler parent;
    event_handler_function destroy_parent_members;
};

typedef struct acceptor_ acceptor;
struct acceptor_ {
    endpoint parent;
    event_handler_function slave;
};

typedef struct endpoint_ connector;

// Generic endpoint
endpoint* endpoint_new(int sock, 
		       event_handler_function handler);
void endpoint_init(endpoint* this, 
		   int sock, 
		   event_handler_function handler);
void endpoint_destroy(endpoint* this);
void endpoint_send(endpoint* this, const void* buf, size_t size);
size_t endpoint_recv(endpoint* this, void* buf, size_t size);


// Stream-oriented server-side endpoints
acceptor* acceptor_new(const char* service,
		       event_handler_function slave);
void acceptor_init(acceptor* h,
		   const char* service,
		   event_handler_function slave);
void acceptor_destroy(acceptor* this);

// Stream-oriented client-side endpoints
endpoint* connector_new(const char* host,
			const char* service,
			event_handler_function handler);
void connector_init(connector* this,
		    const char* host,
		    const char* service,
		    event_handler_function handler);

// Datagram-oriented server-side endpoints
endpoint* udp_endpoint_new(const char* service,
			   event_handler_function handler);
void udp_endpoint_init(endpoint* this,
		       const char* service,
		       event_handler_function handler);

// Datagram-oriented client-side endpoints
endpoint* udp_connector_new(const char* host,
			    const char* service,
			    event_handler_function handler);
void udp_connector_init(endpoint* this,
			const char* host,
			const char* service,
			event_handler_function handler);

#endif
