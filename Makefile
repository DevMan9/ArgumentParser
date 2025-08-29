COMPILER = gcc
FLAGS = -Wall

.PHONY: clean test

run_test: test
	./executable/test
	$(MAKE) clean

test: executable/test

TEST_SOURCE = \
test/test.c\
source/argumentparser.c

executable/test: $(TEST_SOURCE) | executable
	$(COMPILER) $(FLAGS) $(TEST_SOURCE) -o $@

executable:
	mkdir -p $@

clean:
	rm -rf executable