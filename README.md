# Async
Async library based on computed goto for C.

# Features
- Clean syntax, feels almost as native;
- Overhead as little as one ptrdiff_t variable;
- Boilerplate is 0-2 lines of code for every async macro;
- Library is currently around 50 lines of code.

# Disadvantages
- Non ANSI-C compilant. Computed goto is a compiler-specific feature;
- Keep in mind that using every async macro WIPES OUT all local variables;
- # Have fun in debugging this :smiling_face_with_tear:

# Documentation
### Data types
```C
typedef ptrdiff_t async;
```
- This is the only type you need.
async is used in three different contexts:
 - return value of async function;
 - computed goto label;
 - tells if async function has ended.
  	(when function returns its goto label becomes NULL)

### Methods
```C
void async_init(async *self) { *self = (ptrdiff_t)NULL; }
```
- Use this if you want to initialize computed goto label.

### Macroses
```C
#define ASYNC_DISPATCH(state)                                                  \
	void **_state;		   /* Computed goto state. */                  \
	async  _async_call_result; /* For asynchronous calls. */               \
	(void)_async_call_result;                                              \
                                                                               \
	_state = (void *)&state;                                               \
	if (*_state) {                                                         \
		goto **_state;                                                 \
	}
```
- This macros shall be placed at beginning of every async function.
It is used to return to the last saved state. Set 'state' to NON-local variable
of 'async' data type. This variable will then used as computed goto label.

```C
#define ASYNC_YIELD(ret)                                                       \
	do {                                                                   \
		ASYNC_LABEL(return ret)                                        \
	} while (0)
```
- This one is used to return from the function. Further call will continue execution
after this macros. Set 'ret' to any desired value.

```C
#define ASYNC_AWAIT(cond, ret)                                                 \
	do {                                                                   \
		ASYNC_LABEL() if (!(cond)) return ret;                         \
	} while (0)
```
- This macros is the same as 'while (!cond);', but instead of going into
infinite loop, it yields with return value specified in 'ret'.	

```C
#define ASYNC_RETURN(ret)                                                      \
	do {                                                                   \
		*_state = NULL;                                                \
		return ret;                                                    \
	} while (0)
```
- Same as 'return some_value;' but this one is designed for asynchronous function.
Right after this call goto label becomes NULL. which means async function restarts.

```C
#define ASYNC_CALL(func, state)                                                \
	do {                                                                   \
		_async_call_result = func;                                     \
		if (!state)                                                    \
			break;                                                 \
                                                                               \
		ASYNC_YIELD(_async_call_result);                               \
	} while (true)
```
- This allows call asynchronous functions like 'normal' functions within another
asynchronous function. Set 'state' to one which is used by the called function.
The 'state' is used to get acknowledged if nested function used ASYNC_RETURN.
Caller will yield with state that is returned by the callee, except the state
that is returned by ASYNC_RETURN

```C
#define ASYNC_CALL_RESULT _async_call_result
```
- When ASYNC_CALL macros is used, it returns a value, this value then could be
used to check return value of called function.
ASYNC_CALL_RESULT is only valid before any other async macro, since it's local variable.

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

# Shorter version (not yet documented)
```C
#ifndef ASYNC
typedef void * async;
#define ASYNC_CAT1(a, b) a##b
#define ASYNC_CAT(a, b) ASYNC_CAT1(a, b)
#define ASYNC_DISPATCH(state) void **_state = &state; \
			 if (*_state) { goto **_state; }
#define ASYNC_YIELD(act) do { *_state = &&ASYNC_CAT(_l, __LINE__); \
			      act; ASYNC_CAT(_l, __LINE__) :; } while (0)
#define ASYNC_AWAIT(cond, act) \
			 do { ASYNC_YIELD(); if (!(cond)) { act; } } while (0)
#define ASYNC_RESET(act) do { *_state = NULL; act; } while (0)
#define ASYNC
#endif
```
In this version you must specify return value explicitly, for example: ```async_yield(return true)```.
ASYNC_RETURN replaced with ```async_reset(some_action)```.
