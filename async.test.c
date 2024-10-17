#include <stdbool.h> //for bool
#include <time.h>    //for clock_t
#include <stdio.h>   //for printf

#include "async.h"

///////////////////////////////////////////////////////////////////////////////
clock_t get_system_delta_time_ms()
{
	clock_t delta;
	
	static clock_t timestamp_prev = 0;
	static clock_t timestamp      = 0;
	
	timestamp = clock() / (CLOCKS_PER_SEC / 1000);
	delta = timestamp - timestamp_prev;
	timestamp_prev = timestamp;

	return delta;
}

///////////////////////////////////////////////////////////////////////////////
//Print HELLO WORLD every three seconds
struct async_test_hello {
	async async_state;

	int timer;
};

void async_test_hello_init(struct async_test_hello *self)
{
	self->async_state = 0;
	
	self->timer = 0;
}

//Call this function repeatly. It will print "Hello world!" every three seconds
void async_test_hello_update(struct async_test_hello *self, clock_t delta)
{
	//Increase internal timer by delta;
	self->timer += delta;
	
	//Dispatch into the last saved position;
	ASYNC_DISPATCH(self->async_state);
	
	//Await until timer is >= 3000
	ASYNC_AWAIT(self->timer >= 3000, return);

	/* Substract 3000ms from timer.
	   Alternatively set timer to 0 (less precission) */
	self->timer -= 3000;

	printf("Hello world!\n");

	/*Reset routine, (sets state to 0)
	  so it will start from beginning on the next call. */
	ASYNC_RESET(return);
}

///////////////////////////////////////////////////////////////////////////////
//Count from 0 to 10 every 1 second
struct async_test_counter {
	async async_state;

	int counter;
	int timer;
};

void async_test_counter_init(struct async_test_counter *self)
{
	self->async_state = 0;

	self->counter = 0;
	self->timer   = 0;
}

/* Returns true if done counting.
 * Alternatively you could check for
 * self->async_state == 0, if you'd like to return anything else. */
bool async_test_counter_update(struct async_test_counter *self, clock_t delta)
{
	self->timer += delta;
	
	ASYNC_DISPATCH(self->async_state);
	
	for (self->counter = 0; self->counter < 10; self->counter++) {
		printf("counter: %i\n", self->counter);
		fflush(0);

		ASYNC_AWAIT(self->timer >= 1000, return false);

		self->timer -= 1000;
	}

	ASYNC_RESET(return true);
}

///////////////////////////////////////////////////////////////////////////////
struct async_test_hello   hello;
struct async_test_counter counter;
static async async_test_main_state;

void async_test_main()
{
	clock_t delta = get_system_delta_time_ms();

	ASYNC_DISPATCH(async_test_main_state);

	async_test_hello_init(&hello);
	async_test_counter_init(&counter);
	
	do {
		//Call hello
		async_test_hello_update(&hello, delta);

		//If async_test_counter ends - exit loop
		if (async_test_counter_update(&counter, delta))
			break;

		/* WARNING!
		 * Every infinite loop must be yielded at some point
		 * Otherwise it will never return and system will fall
		   Into a deadlock. */
		ASYNC_YIELD(return);
	} while (true);

	ASYNC_RESET(return);
}

///////////////////////////////////////////////////////////////////////////////
int main()
{	
	do {
		//Call async_test_main repeatly
		async_test_main();
	} while (async_test_main_state != 0); //Until it resets (ASYNC_RESET)
	
	return 0;
}
