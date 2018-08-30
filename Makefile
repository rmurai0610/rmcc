CC=gcc -g -O0
OBJS=error-util.o debug.o vector.o map.o lexer.o parser.o symbol-table.o codegen.o
PROGRAM=rmcc
UTILS_TEST=utils-test

all: $(PROGRAM)

$(PROGRAM): $(PROGRAM).o $(OBJS)
	$(CC) $(PROGRAM).o $(OBJS) -o $(PROGRAM)

$(UTILS_TEST): $(UTILS_TEST).o $(OBJS)
	$(CC) $(UTILS_TEST).o $(OBJS) -o $(UTILS_TEST)

test: $(UTILS_TEST)
	./test.sh
	./utils-test

clean:
	rm -f $(PROGRAM) $(UTILS_TEST) *.o *.out *.s
