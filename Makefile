CC=c++
FLAG=-Wall -std=c++14 -I./include/range-v3/include -I./include/catch/single_include -I./include/fmt
MAIN=main
TEST=test-exec
OBJS=type.o ast.o checker.o main.o

.PHONY: test clean all

build: $(OBJS)

all: $(OBJS) test main

type.o:
	$(CC) $(FLAG) -c ./src/type.hpp

ast.o:
	$(CC) $(FLAG) -c ./src/ast.hpp

checker.o:
	$(CC) $(FLAG) -c ./src/checker.hpp

main.o: type.o
	$(CC) $(FLAG) -c ./src/main.cc

main: main.o
	$(CC) $(FLAG) main.o -o $(MAIN)

test:
	$(CC) $(FLAG) ./test/test.cc -o $(TEST) && ./$(TEST)

clean:
	rm *.o $(MAIN) $(TEST)
