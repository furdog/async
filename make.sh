mkdir -p build

input="async.test.c"
output="build/test"

rm ${output}
gcc ${input} -Wall -Wextra -g -o ${output}

./${output}
read -p "Done. Press ENTER to exit"
