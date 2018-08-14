CC=gcc
OBJS=error-util.o vector.o lexer.o parser.o codegen.o
PROGRAM=self-c
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
