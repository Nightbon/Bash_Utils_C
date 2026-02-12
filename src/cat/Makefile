CC = gcc
CFLAGS = -Wall -Wextra -Werror

.PHONY: all clean test

all: s21_cat

s21_cat: s21_cat.o error.o is_binary_file.o
	$(CC) $(CFLAGS) s21_cat.o error.o is_binary_file.o -o s21_cat

error.o: ../common/error.c ../common/error.h
	$(CC) $(CFLAGS) -c ../common/error.c

is_binary_file.o: ../common/is_binary_file.c ../common/is_binary_file.h
	$(CC) $(CFLAGS) -c ../common/is_binary_file.c

s21_cat.o: s21_cat.c s21_cat.h ../common/error_codes.h
	$(CC) $(CFLAGS) -c s21_cat.c

clean:
	rm -rf *.o s21_cat
	rm -rf test_data output expected cat

test: s21_cat
	./run_tests.sh