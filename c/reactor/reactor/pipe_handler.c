#define _GNU_SOURCE
#include <reactor/pipe_handler.h>
#include <reactor/exception.h>
#include <reactor/reactor.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

static void pipe_handler_free_members(pipe_handler* h);

pipe_handler* pipe_handler_new (event_handler_function handler)
{
    pipe_handler* h = malloc(sizeof(pipe_handler));
    pipe_handler_init (h, handler);
    h->parent.destroy_self = (event_handler_function) free;
    return h;
}

void pipe_handler_init (pipe_handler* h, event_handler_function handler)
{
    pipe2(h->pipe, O_DIRECT);
    event_handler_init (&h->parent, h->pipe[0], handler);
    h->destroy_parent_members = h->parent.destroy_members;
    h->parent.destroy_members = (event_handler_function) pipe_handler_free_members;
}

void pipe_handler_destroy (pipe_handler* ev)
{
    event_handler_destroy(&ev->parent);
}

int pipe_handler_write_ne (pipe_handler* h, const void* buf, size_t size)
{
    return write(h->pipe[1], buf, size);
}

int pipe_handler_read_ne (pipe_handler* h, void* buf, size_t max_size)
{
    return read(h->pipe[0], buf, max_size);
}

void pipe_handler_write (pipe_handler* h, const void* buf, size_t size)
{
    if (size > PIPE_BUF)
	Throw Exception(EINVAL, "Tamaño demasiado grande");
    int n = write(h->pipe[1], buf, size);
    if (0 > n)
	Throw Exception(errno, "Imposible escribir");
}

int pipe_handler_read (pipe_handler* h, void* buf, size_t max_size)
{
    int n = read(h->pipe[0], buf, max_size);
    if (0 > n)
	Throw Exception(errno, "Imposible leer");
    return n;
}

static void pipe_handler_free_members(pipe_handler* h)
{
    event_handler* ev = &h->parent;
    if (ev->r)
	reactor_disable(ev->r, ev->fd);
    close(h->pipe[0]); close(h->pipe[1]);
    h->destroy_parent_members(&h->parent);
}
