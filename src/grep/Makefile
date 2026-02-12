CC = gcc
CFLAGS = -Wall -Wextra -Werror

.PHONY: all clean test

all: s21_grep

s21_grep: s21_grep.o error.o is_binary_file.o
	$(CC) $(CFLAGS) s21_grep.o error.o is_binary_file.o -o s21_grep

error.o: ../common/error.c ../common/error.h
	$(CC) $(CFLAGS) -c ../common/error.c

is_binary_file.o: ../common/is_binary_file.c ../common/is_binary_file.h
	$(CC) $(CFLAGS) -c ../common/is_binary_file.c

s21_grep.o: s21_grep.c s21_grep.h ../common/error_codes.h
	$(CC) $(CFLAGS) -c s21_grep.c

clean:
	rm -rf *.o s21_grep
	rm -rf test_data output expected grep

test: s21_grep
	./run_tests.sh