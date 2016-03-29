#include "event_handler.h"
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
	ev->handle_events(ev);
	assert(calls == 1);
	ev->destroy(ev);
	return 0;
}