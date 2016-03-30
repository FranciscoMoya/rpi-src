#include <reactor/event_handler.h>
#include <assert.h> /* assert */
#include <stdlib.h> /* NULL */

int main() 
{
	event_handler* ev = NULL;
	int calls = 0;

	void fn(event_handler* e) { 
		assert(ev == e); ++calls; 
	}

	ev = event_handler_new(-1, fn);
	assert(ev != NULL);
	event_handler_handle_events(ev);
	event_handler_handle_events(ev);
	assert(calls == 2);
	event_handler_destroy(ev);
	return 0;
}
