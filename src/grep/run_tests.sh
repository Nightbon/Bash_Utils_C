#!/bin/bash
cp s21_grep grep
set -e

TEST_DATA_DIR="test_data"
EXPECTED_DIR="expected"
OUTPUT_DIR="output"
mkdir -p $EXPECTED_DIR $OUTPUT_DIR $TEST_DATA_DIR

######################################### создание тестовых файлов #############################################
cat > $TEST_DATA_DIR/file1.txt <<EOF
Hello World
TEST pattern
another line
12345
aaaaa
EOF

cat > $TEST_DATA_DIR/file2.txt <<EOF
Another 789 line
OnlyText
aaa
test test
EOF

echo "Hello" > $TEST_DATA_DIR/patterns.txt
echo -e "abc\x00def\x00pattern\x00ghi" > $TEST_DATA_DIR/binary_test.bin
echo -e "\x00\x01\x02test" > $TEST_DATA_DIR/binary2.bin
echo "test" > $TEST_DATA_DIR/empty_line.txt
echo -n "no newline" > $TEST_DATA_DIR/no_newline.txt
echo -e "Hello\nTEST" > $TEST_DATA_DIR/multi_pattern.txt
echo "" > $TEST_DATA_DIR/empty_pattern.txt
echo "ThisIsAReallyLongLineWithPattern_$(printf '%*s' 5000 | tr ' ' 'A')" > $TEST_DATA_DIR/long_line.txt
echo "$(printf '%*s' 5000 | tr ' ' 'A')" > $TEST_DATA_DIR/long_pattern.txt
echo "test test test" > $TEST_DATA_DIR/multi_match.txt
echo -n -e "\x00\x00\x00" > $TEST_DATA_DIR/all_null.bin
echo -e "text\x00with\x00null" > $TEST_DATA_DIR/null_bytes.txt
echo "СЪешь ещё этих мягких французских булок" > $TEST_DATA_DIR/unicode.txt
touch $TEST_DATA_DIR/empty.txt
cp $TEST_DATA_DIR/file1.txt "$TEST_DATA_DIR/file with spaces.txt"
chmod 000 $TEST_DATA_DIR/protected.txt 2>/dev/null || true

######################################### функция тестирования #######################################
run_test() {
    local test_name=$1
    local grep_args="$2"
    echo -n "Running $test_name..."
    
    grep $grep_args > "$EXPECTED_DIR/${test_name}_expected.txt" 2>&1 || true
    ./grep $grep_args > "$OUTPUT_DIR/${test_name}_output.txt" 2>&1 || true
    
    diff -u "$EXPECTED_DIR/${test_name}_expected.txt" "$OUTPUT_DIR/${test_name}_output.txt" || exit 1

    echo -e "\033[32mOK!\033[0m"
}
echo -e "\n"
######################################### Основные флаги #############################################
run_test "without_flags" "Hello $TEST_DATA_DIR/file1.txt"
run_test "e_pattern" "Hello $TEST_DATA_DIR/file1.txt"
run_test "i_ignore_case" "-i hello $TEST_DATA_DIR/file1.txt"
run_test "v_invert_match" "-v Test $TEST_DATA_DIR/file1.txt"
run_test "c_count" "-c [0-9] $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
run_test "l_files_with_matches" "-l a* $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
run_test "n_line_numbers" "-n pattern $TEST_DATA_DIR/file1.txt"
######################################### дополнительные флаги #######################################
run_test "h_no_filename" "-h test $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
run_test "s_suppress_errors" "-s invalid_file $TEST_DATA_DIR/file1.txt"
run_test "f_file_pattern" "-f $TEST_DATA_DIR/patterns.txt $TEST_DATA_DIR/file1.txt"
run_test "o_multiple_matches" "-o test $TEST_DATA_DIR/file2.txt"
######################################### Комбинации флагов ###########################################
run_test "e_and_e" "-e Hello -e TEST $TEST_DATA_DIR/file1.txt"
run_test "v_and_o" "-v -o Hello $TEST_DATA_DIR/file1.txt"
run_test "c_and_l" "-c -l test $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
run_test "o_and_n" "-n -o a $TEST_DATA_DIR/file1.txt"
run_test "o_and_v" "-v -o test $TEST_DATA_DIR/file2.txt"
run_test "o_and_c" "-o -c a $TEST_DATA_DIR/file1.txt"
run_test "o_and_f" "-o -f $TEST_DATA_DIR/multi_pattern.txt $TEST_DATA_DIR/file1.txt"
run_test "l_and_n" "-l -n test $TEST_DATA_DIR/file2.txt"
run_test "h_and_l" "-h -l test $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
run_test "h_and_n" "-h -n test $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
run_test "f_and_v" "-v -f $TEST_DATA_DIR/patterns.txt $TEST_DATA_DIR/file1.txt"
run_test "s_and_l" "-s -l test $TEST_DATA_DIR/invalid.txt $TEST_DATA_DIR/file1.txt"
run_test "s_and_h" "-s -h test $TEST_DATA_DIR/invalid.txt $TEST_DATA_DIR/file1.txt"
run_test "e_end_f" "-e Hello -f $TEST_DATA_DIR/multi_pattern.txt $TEST_DATA_DIR/file1.txt"
run_test "v_and_c" "-v -c test $TEST_DATA_DIR/file2.txt"
run_test "i_n_o_combination" "-i -n -o test $TEST_DATA_DIR/file1.txt"
run_test "c_l_h_combination" "-c -l -h test $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
run_test "i_v_c_combination" "-i -v -c TEST $TEST_DATA_DIR/file1.txt"
run_test "c_o_l_combination" "-c -o -l test $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
run_test "all_combination" "-ivnscolh -e test -f $TEST_DATA_DIR/patterns.txt $TEST_DATA_DIR/file1.txt $TEST_DATA_DIR/file2.txt"
######################################### Краевые случаи ###########################################
run_test "long_line" "Pattern $TEST_DATA_DIR/long_line.txt"
run_test "long_pattern" "-f $TEST_DATA_DIR/long_pattern.txt $TEST_DATA_DIR/long_line.txt"
run_test "empty_pattern" "-f $TEST_DATA_DIR/empty_pattern.txt $TEST_DATA_DIR/file1.txt"
run_test "multi_pattern" "-f $TEST_DATA_DIR/multi_pattern.txt $TEST_DATA_DIR/file1.txt"
run_test "binary_test" "abc $TEST_DATA_DIR/binary_test.bin"
run_test "invalid_pattern_file" "-f invalid.txt $TEST_DATA_DIR/file1.txt"
run_test "no_newline" "no $TEST_DATA_DIR/no_newline.txt"
run_test "special_chars" "-e ^H[a-z] $TEST_DATA_DIR/file1.txt"
run_test "empty_file" "pattern $TEST_DATA_DIR/empty.txt"
run_test "invalid_regex" "-e [[invalid $TEST_DATA_DIR/file1.txt"
run_test "empty_matches" "-o () $TEST_DATA_DIR/file1.txt"
run_test "multiple_v" "-v -v test $TEST_DATA_DIR/file1.txt"
run_test "escaped_chars" "-e "Hello\\ World" $TEST_DATA_DIR/file1.txt"
run_test "spaced_filename" "Hello $TEST_DATA_DIR/file with spaces.txt"
run_test "multiple_binary" "abc $TEST_DATA_DIR/binary_test.bin $TEST_DATA_DIR/binary2.bin"
run_test "escaped_e" "-e "Hello\\ World" -e TEST $TEST_DATA_DIR/file1.txt"
run_test "empty_file_v" "-v -f $TEST_DATA_DIR/empty_pattern.txt $TEST_DATA_DIR/file1.txt"
run_test "null_bytes" "text $TEST_DATA_DIR/null_bytes.txt"
run_test "mixed_file_types" "test $TEST_DATA_DIR/file2.txt $TEST_DATA_DIR/binary_test.bin"
run_test "regex_overflow" "-e .*a.*b.*c.*d.*e $TEST_DATA_DIR/file1.txt"
run_test "backreference" "-e \(a\).*\1 $TEST_DATA_DIR/file1.txt"
run_test "multi_match_line" "-o test $TEST_DATA_DIR/multi_match.txt"
run_test "all_null_file" "pattern $TEST_DATA_DIR/all_null.bin"
run_test "unreadable_file" "-s test $TEST_DATA_DIR/protected.txt"
run_test "edge_pattern" "-e ^a*$ $TEST_DATA_DIR/file1.txt"
run_test "unicode_case" "-i -o съешь $TEST_DATA_DIR/unicode.txt"

echo -e "\n"
######################################### Тест на стиль ###########################################
echo -n "Running clang-format..."
if output=$(clang-format -n *.c *.h 2>&1) && [ -z "$output" ]; then
    echo -e "\033[32m OK!\033[0m"
else
    echo -e "\033[31m FAILED!\033[0m"
    echo "Formatting issues found:"
    echo "$output"
    exit 1
fi
######################################### Статический анализатор ###################################
echo -n "Running cppcheck..."
if cppcheck --enable=all --suppress=missingIncludeSystem s21_grep.c >/dev/null 2>&1; then
    echo -e "\033[32m OK!\033[0m"
else
    echo -e "\033[31m FAILED!\033[0m"
    echo "Cppcheck issues found:"
    cppcheck --enable=all --suppress=missingIncludeSystem s21_grep.c
    exit 1
fi
######################################### Тест памяти ########################################
echo -n "Running valgrind..."
output=$(valgrind --leak-check=full ./grep Hello $TEST_DATA_DIR/file1.txt 2>&1)
if ! echo "$output" | grep -q -e "ERROR SUMMARY: 0 errors" -e "no leaks are possible"; then
    echo -e "\033[31m FAILED!\033[0m"
    echo "Memory issues found:"
    echo "$output"
    exit 1
else
    echo -e "\033[32m OK!\033[0m"
fi

echo -e "\n\033[1;32mAll tests passed!\033[0m"