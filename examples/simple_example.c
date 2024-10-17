#include <stdbool.h>
#include <time.h>
#include <stdio.h>

#include "async.h"

clock_t global_timestamp;

void async_test_hello_update()
{
	static async   async_state = 0;
	static clock_t timestamp   = 0;
	
	ASYNC_DISPATCH(async_state);
	
	ASYNC_AWAIT(global_timestamp - timestamp >= 3000, return);
	timestamp = global_timestamp;

	printf("Hello world!\n");

	ASYNC_RESET(return);
}

bool async_test_counter_update()
{
	static async   async_state = 0;
	static clock_t timestamp   = 0;
	static int     counter     = 0;
	
	ASYNC_DISPATCH(async_state);
	
	for (counter = 0; counter < 10; counter++) {
		printf("counter: %i\n", counter);
		fflush(0);

		ASYNC_AWAIT(global_timestamp - timestamp >= 1000,
			    return false);
		timestamp = global_timestamp;
	}

	ASYNC_RESET(return true);
}

int main()
{	
	do {
		global_timestamp = clock() / (CLOCKS_PER_SEC / 1000);

		async_test_hello_update();
		if (async_test_counter_update())
			break;
	} while (true);
	
	return 0;
}
