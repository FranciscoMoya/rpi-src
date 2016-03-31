#define _GNU_SOURCE
#include <reactor/spawn_handler.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

static void spawn_handler_free_members(event_handler* ev);

spawn_handler* spawn_handler_new (event_handler_function parent_handle,
                                  event_handler_function child_handle)
{
    spawn_handler* h = malloc(sizeof(spawn_handler));
    spawn_handler_init(h, parent_handle, child_handle);
    h->parent.destroy_self = (event_handler_function)free;
    return h;
}


void spawn_handler_init (spawn_handler* h,
                              event_handler_function parent_handle,
                              event_handler_function child_handle)
{
    int parent_to_child[2], child_to_parent[2];

    pipe2(parent_to_child, O_DIRECT);
    pipe2(child_to_parent, O_DIRECT);
    h->pid = fork();
    if (spawn_handler_is_child(h)) {
        event_handler_init(&h->parent, parent_to_child[0], child_handle);
        h->out = child_to_parent[1];
        close(parent_to_child[1]); close(child_to_parent[0]);
    }
    else {
        event_handler_init(&h->parent, child_to_parent[0], parent_handle);
        h->out = parent_to_child[1];
        close(child_to_parent[1]); close(parent_to_child[0]);
    }
    h->destroy_parent_members = h->parent.destroy_members;
    h->parent.destroy_members = spawn_handler_free_members;
}


void spawn_handler_destroy(spawn_handler* ev)
{
    event_handler_destroy(&ev->parent);
}


int spawn_handler_is_child (spawn_handler* ev)
{
    return 0 == ev->pid;
}


void spawn_handler_stay_forever_on_child (spawn_handler* ev)
{
    if (!spawn_handler_is_child(ev))
        return;

    reactor* r = reactor_new();
    reactor_add(r, &ev->parent);
    reactor_run(r);
    reactor_destroy(r);
    exit(0);
}

static void spawn_handler_free_members(event_handler* ev)
{
    spawn_handler* h = (spawn_handler*)ev;

    close(h->out); close(h->parent.fd);
    if (h->pid) { // parent
        kill(h->pid, SIGTERM);
        waitpid(h->pid, NULL, 0);
    }
    h->destroy_parent_members(ev);
}
