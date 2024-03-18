CC ?= gcc
CFLAG = -Wall -Wextra -Wno-missing-braces -std=gnu17 -g 
LIB = -lpthread -fsanitize=leak,bounds
SRC = src/*.c

.PHONY: static test

chan: $(SRC) 
	$(CC) $(CFLAG) -shared -fPIC $(SRC) -o lib$@.so $(LIB)

static: $(SRC) 
	$(CC) $(CFLAG) $(SRC) -c

clean: 
	rm *.o *.so *.dll test

test: test.c 
	$(CC) $(CFLAG) channel.o $< -o $@ $(LIB) -fopenmp
