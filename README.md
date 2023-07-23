# Async
Async library based on computed goto for C.

# Features
- Clean syntax, feels almost as native;
- Overhead as little as one ptrdiff_t variable;
- Boilerplate is 0-2 lines of code for every async macro;
- Library is currently around 50 lines of code.

# Basics

```C
#include "async.h"
#include <stdio.h>

async basic_example()
{
	/* Every asynchronous macro destroy local variables.
	 * So we need to keep them non-local, or pass as *self parameter. */
	static async state;
	static int i;
	
	/* Dispatcher will bring us back to the last yielded state. */
	ASYNC_DISPATCH(state);
	
	for (i = 0; i < 5; i++) {
		printf("Value of i variable: %i\n", i);

		/* Return from function with return value of 1.
		 * When function gets called again, dispatcher will
		 * continue executing code after this yield.*/
		ASYNC_YIELD(1);
	}
	
	/* Return 0, so we know function has ended. */
	ASYNC_RETURN(0);
}

int main()
{
	/* Rerun basic_exaple until it returns 0. */
	while (basic_example());

	return 0;
}
```

# More complicated example
This is an example of simple hex-string parser for streaming data.
The original code is intended to convert serial data incoming from serial port to CAN data,
which is then converted to actual vehicle CAN frames. You can learn more about this by looking
test cases and testing code. The code below is not complete, it only demonstrates 2 variants of same function.

- Native variant:
```C
bool parse_line(struct instance *self, const char **str)
{
	/* If parsed unsuccesfully. */
	if (!tokenize_hex(self, str)) {
		skip_until_eol(str);
		return false;
	}

	/* Set CAN id. */
	self->can.id = strtol(self->hex_str, NULL, 16);

	/* Set CAN data. */
	for (self->can.len = 0; self->can.len < 8; self->can.len++) {
		if (!tokenize_hex(self, str)) {
			skip_until_eol(str);
			return true;
		}

		self->can.data[self->can.len] = strtol(self->hex_str, NULL, 16);
	}

	/* Skip the rest of line. */
	skip_until_eol(str);
	return true;
}
```

- Asynchronous variant:
```C
async parse_line(struct instance *self)
{
	ASYNC_DISPATCH(self->state);

	/* If parsed unsuccesfully. */
	ASYNC_CALL(tokenize_hex(self), self->sub_state);
	if (ASYNC_CALL_RESULT == RET_FAIL) {
		ASYNC_CALL(skip_until_eol(self), self->sub_state);
		ASYNC_RETURN(RET_FAIL);
	}

	/* Set CAN id. */
	self->can.id = strtol(self->hex_str, NULL, 16);

	/* Set CAN data. */
	for (self->can.len = 0; self->can.len < 8; self->can.len++) {
		ASYNC_CALL(tokenize_hex(self), self->sub_state);
		if (ASYNC_CALL_RESULT == RET_FAIL) {
			ASYNC_CALL(skip_until_eol(self), self->sub_state);
			ASYNC_RETURN(RET_OK);
		}

		self->can.data[self->can.len] = strtol(self->hex_str, NULL, 16);
	}

	/* Skip the rest of line. */
	ASYNC_CALL(skip_until_eol(self), self->sub_state);
	ASYNC_RETURN(RET_OK);
}
```
As you can see, in order to asynchronous code to work we did almost no changes to the original code,
except changed the way we reference things and syntax became little more complicated.

# Rationale
The main disadvantage of native variant is that in order to convert serial data to CAN
We must have access to full serial string. But serial data as known is non-synchronous and have
streaming nature, we cannot just run native code without having full string available. 
Moreover we do not know the lengh of buffer were gonna parse at the first place.

But... What if we could just run native function, and force it to wait for incoming data?
Well we can do that, but then function will block other code, which is not always desired.

With async you could do that with minimum efforts.

# Further notes
The library not in its final state.
The main point is to keep is as minimal as posible.
