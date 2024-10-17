# Async
Async library based on computed goto for C.

# Features
- The library contains 13 lines of code at the moment.

# Disadvantages
- Non ANSI-C compilant. Computed goto is a compiler-specific feature;
- Reusing local variables from previous call is considered undefined behaviour.

# Usage
The general idea - exit from function
and upon call return to the last exit point.

#
```C
ASYNC_DISPATCH(state_variable of type `async`)
```
CAN be placed at the beggining of a function. It jumps to the previous saved point.

#
```C
ASYNC_YIELD(action)
```
Save point of execution that lies after `action`.
The `action` may be generally anything, for example `return` operator.

#
```C
ASYNC_AWAIT(condition, action)
```
Awaits for condition to be true. If false, it calls `ASYNC_YIELD` with action.

#
```C
ASYNC_RESET(action)
```
Resets point of execution to ZERO, performs some `action` before that.

#
The default `action` is the `return` operator. This is intended by design,
But it might be anything else.

# Examples
See example files.
The expected output is:
```
counter: 0
counter: 1
counter: 2
Hello world!
counter: 3
counter: 4
counter: 5
Hello world!
counter: 6
counter: 7
counter: 8
Hello world!
counter: 9
Done. Press ENTER to exit
```
**simple_example.c** - Singleton. does exactly what it needed to get the
desired output. It might be simple to read for human beings, it has no comments and any extra logic.

**complex_example.c** - Object oriented model with comments and
extra logic. It represents etalon way to work with the async library.
