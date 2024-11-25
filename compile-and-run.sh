rm compiler
gcc compiler.c lexico.c parser.c semantic.c -o compiler -Wall -Wextra -fsanitize=address -g -fsanitize=undefined -fstack-protector -Werror
./compiler