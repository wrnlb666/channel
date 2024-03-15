CC ?= clang
CFLAG = -Wall -Wextra -std=gnu17 -g 
LIB = -lpthread -fsanitize=leak,bounds
SRC = src/*.c

chan: $(SRC) 
	$(CC) $(CFLAG) -shared -fPIC $(SRC) -o lib$@.so $(LIB)

static: $(SRC) 
	$(CC) $(CFLAG) $(SRC) -c


test: test.c 
	$(CC) $(CFLAG) channel.o $< -o $@ $(LIB) -fopenmp
