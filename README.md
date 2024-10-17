# Async
Async library based on computed goto for C.

# Features
- Clean syntax, feels almost as native;
- Overhead as little as one ptrdiff_t variable;
- Boilerplate is 0-2 lines of code for every async macro;

# Disadvantages
- Non ANSI-C compilant. Computed goto is a compiler-specific feature;
- Keep in mind that using every async macro WIPES OUT all local variables;
- # Have fun in debugging this :smiling_face_with_tear:

# Usage
You could find example usage in test file.
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
