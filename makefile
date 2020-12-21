CC = g++
CFLAGS = -std=c++14 -Wall -Wextra -Werror
EXEC = solver.out

all: clean
all: $(EXEC)

debug: CFLAGS += -DDEBUG
debug: clean
debug: $(EXEC)

$(EXEC): main.o solver.o
	    $(CC) $(CFLAGS) -o solver.out main.o solver.o

main.o: main.cpp solver.hpp
	   $(CC) $(CFLAGS) -c main.cpp

solver.o: solver.cpp solver.hpp
	     $(CC) $(CFLAGS) -c solver.cpp

clean:
	     rm -f solver.out *.o
