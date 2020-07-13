CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c99 -g

BINARIES = ruterdrift

all: $(BINARIES)


run5:
	./$(BINARIES) 5_routers_fully_connected

run10:
	./$(BINARIES) 50_routers_150_edges kommandoer_10_routers.txt

run50:
	./$(BINARIES) 50_routers_150_edges kommandoer_50_rutere.txt

clean:
	rm -f $(BINARIES)

val:
	valgrind --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes ./$(BINARIES)
