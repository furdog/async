mkdir -p build

input="examples/complex_example.c"
#input="examples/simple_example.c"
output="build/test"

rm ${output}
gcc ${input} -I"$PWD" -Wall -Wextra -g -o ${output}

./${output}
read -p "Done. Press ENTER to exit"
