#!/bin/bash
cp s21_cat cat
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

cat > $TEST_DATA_DIR/empty_lines.txt <<EOF
line1

line2



line3
EOF

printf "Hello\tWorld\nline with tab\tand newline\nline with \x01\x02\x03\n" > $TEST_DATA_DIR/tabs_and_specials.txt
printf "\t\t\t\n" > $TEST_DATA_DIR/only_tabs.txt
echo -e "This is a very long line: $(printf '%*s' 5000 | tr ' ' 'A')" > $TEST_DATA_DIR/long_line.txt
echo -n -e "text\x00with\x00null\x01\x02\x03" > $TEST_DATA_DIR/binary_data.bin
echo "СЪешь ещё этих мягких французских булок" > $TEST_DATA_DIR/unicode.txt
echo -n "no newline" > $TEST_DATA_DIR/no_newline.txt
touch $TEST_DATA_DIR/empty.txt
touch $TEST_DATA_DIR/protected.txt
chmod 000 $TEST_DATA_DIR/protected.txt 2>/dev/null || true

######################################### функция тестирования #######################################
run_test() {
    local test_name=$1
    local cat_args="$2"
    echo -n "Running $test_name..."
    
    cat $cat_args > "$EXPECTED_DIR/${test_name}_expected.txt" 2>&1 || true
    ./cat $cat_args > "$OUTPUT_DIR/${test_name}_output.txt" 2>&1 || true
    
    diff -u "$EXPECTED_DIR/${test_name}_expected.txt" "$OUTPUT_DIR/${test_name}_output.txt" || exit 1

    echo -e "\033[32mOK!\033[0m"
}

echo -e "\n"
######################################### Основные флаги #############################################
run_test "without_flags" "$TEST_DATA_DIR/file1.txt"
run_test "b_number_nonblank_short" "-b $TEST_DATA_DIR/empty_lines.txt $TEST_DATA_DIR/file1.txt"
run_test "number_nonblank_long" "--number-nonblank $TEST_DATA_DIR/empty_lines.txt $TEST_DATA_DIR/file1.txt"
run_test "e_show_ends_v_specials" "-e $TEST_DATA_DIR/tabs_and_specials.txt"
run_test "E_show_ends_no_v_specials" "-E $TEST_DATA_DIR/tabs_and_specials.txt"
run_test "n_number_all_short" "-n $TEST_DATA_DIR/empty_lines.txt $TEST_DATA_DIR/file1.txt"
run_test "number_all_long" "--number $TEST_DATA_DIR/empty_lines.txt $TEST_DATA_DIR/file1.txt"
run_test "s_squeeze_blank_short" "-s $TEST_DATA_DIR/empty_lines.txt"
run_test "squeeze_blank_long" "--squeeze-blank $TEST_DATA_DIR/empty_lines.txt"
run_test "t_show_tabs_v" "-t $TEST_DATA_DIR/tabs_and_specials.txt"
run_test "T_show_tabs_no_v_" "-T $TEST_DATA_DIR/tabs_and_specials.txt"
######################################### Комбинации флагов #############################################
run_test "b_and_s" "-b -s $TEST_DATA_DIR/empty_lines.txt"
run_test "b_and_E" "-b -E $TEST_DATA_DIR/file1.txt"
run_test "b_and_T" "-b -T $TEST_DATA_DIR/tabs_and_specials.txt"
run_test "n_and_s" "-n -s $TEST_DATA_DIR/empty_lines.txt"
run_test "n_and_T" "-n -T $TEST_DATA_DIR/tabs_and_specials.txt"
run_test "n_and_E" "-n -E $TEST_DATA_DIR/file1.txt"
run_test "s_and_T" "-s -T $TEST_DATA_DIR/tabs_and_specials.txt $TEST_DATA_DIR/empty_lines.txt"
run_test "E_and_t" "-E -t $TEST_DATA_DIR/tabs_and_specials.txt $TEST_DATA_DIR/empty_lines.txt"
run_test "e_and_t" "-e -t $TEST_DATA_DIR/tabs_and_specials.txt $TEST_DATA_DIR/empty_lines.txt"
run_test "e_and_number-nonblank" "-e --number-nonblank $TEST_DATA_DIR/tabs_and_specials.txt $TEST_DATA_DIR/empty_lines.txt"
run_test "e_and_number" "-e --number $TEST_DATA_DIR/tabs_and_specials.txt $TEST_DATA_DIR/empty_lines.txt"
run_test "t_and_squeeze-blank" "-t --squeeze-blank $TEST_DATA_DIR/tabs_and_specials.txt $TEST_DATA_DIR/empty_lines.txt"
######################################### Краевые случаи #############################################
run_test "basic_long_line" "$TEST_DATA_DIR/long_line.txt"
run_test "nonexistent_file" "nonexistent.txt"
run_test "protected_file" "$TEST_DATA_DIR/protected.txt"
run_test "only_tabs" "-T $TEST_DATA_DIR/only_tabs.txt"
run_test "empty_file" "$TEST_DATA_DIR/empty.txt"
run_test "no_newline" "$TEST_DATA_DIR/no_newline.txt"
run_test "unicode_case" "$TEST_DATA_DIR/unicode.txt"
run_test "binary_data" "$TEST_DATA_DIR/binary_data.bin"

echo -e "\n"
######################################### Тест на стиль ###########################################
echo -n "Running clang-format check..."
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
if cppcheck --enable=all --suppress=missingIncludeSystem s21_cat.c >/dev/null 2>&1; then
    echo -e "\033[32m OK!\033[0m"
else
    echo -e "\033[31m FAILED!\033[0m"
    echo "Cppcheck issues found:"
    cppcheck --enable=all --suppress=missingIncludeSystem s21_cat.c
    exit 1
fi
######################################### Тест памяти ########################################
echo -n "Running valgrind..."
output=$(valgrind --leak-check=full ./cat $TEST_DATA_DIR/long_line.txt 2>&1)
if ! echo "$output" | grep -q -e "ERROR SUMMARY: 0 errors" -e "no leaks are possible"; then
    echo -e "\033[31m FAILED!\033[0m"
    echo "Memory issues found:"
    echo "$output"
    exit 1
else
    echo -e "\033[32m OK!\033[0m"
fi

echo -e "\n\033[1;32mAll tests passed!\033[0m\n"