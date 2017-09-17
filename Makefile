CC = gcc
CFLAGS = -g
LIBS = .
SRC = main.c 
OBJ = $(SRC:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

sim:
	$(CC) $(CFLAGS) src/main.c -o bin/sim

clean :
	@rm bin/sim
	@echo Cleaned!
