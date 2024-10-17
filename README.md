# Async
Async library based on computed goto for C.

# Features
- Clean syntax, feels almost as native;
- Super small library, 13 lines of code at the moment, LOL;
- Overhead as little as one ptrdiff_t variable;
- Boilerplate is 0-2 lines of code for every async macro;

# Disadvantages
- Non ANSI-C compilant. Computed goto is a compiler-specific feature;
- Keep in mind that using every async macro WIPES OUT all local variables;
- ## Have fun in debugging this :smiling_face_with_tear:

# Usage
The general idea is extremely simple - exit from function
and upon call return to the last exit point.

#
```C
ASYNC_DISPATCH(state_variable of type `async`)
```
CAN be placed at the beggining of function. It jumps to the previous saved point.

#
```C
ASYNC_YIELD(action)
```
Save point of execution that lies after `action`.
`action` may be generraly anything, for example `return` operator.

#
```C
ASYNC_AWAIT(condition, action)
```
Awaits for condition to be true. If false, it calls ASYNC_YIELD with action.

#
```C
ASYNC_RESET(action)
```
Resets point of execution to ZERO, performs some action before that.

#
the default `action` is the `return` operator. This is intended by design,
But it might be anything else. Who knows what your sick mind might imagine.

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
desired output. It's simple to read, it has no comments and any extra logic.

**complex_example.c** - it's more deep, object oriented model with comments and
extra stuff. It's shows the best way to work with the async library.
