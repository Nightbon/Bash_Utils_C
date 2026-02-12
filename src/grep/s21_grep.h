#ifndef S21_GREP_H
#define S21_GREP_H

#include <libgen.h>
#include <locale.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../common/error.h"
#include "../common/error_codes.h"
#include "../common/is_binary_file.h"

#define MAX_ERROR_MSG 256  ///< Максимальная длина сообщения об ошибке regcomp

/**
 * @brief Структура для хранения параметров программы
 */
typedef struct {
  regex_t *regexes;  ///< Массив скомпилированных регулярных выражений
  size_t num_regexes;  ///< Количество скомпилированных выражений
  char **patterns;  ///< Массив строковых шаблонов для поиска
  size_t num_patterns;  ///< Количество шаблонов
  int ignore_case;  ///< Флаг игнорирования регистра (-i)
  int invert_match;  ///< Флаг инвертирования совпадений (-v)
  int count_only;  ///< Вывод только количества совпадений (-c)
  int files_with_matches;  ///< Вывод только имен файлов с совпадениями (-l)
  int line_number;         ///< Вывод номеров строк (-n)
  int print_filename;  ///< Вывод имени файла перед результатами
  int output_the_matched;  ///< Вывод только совпавших частей (-o)
  int suppress_error;          ///< Подавление ошибок (-s)
  int print_without_filename;  ///< Запрет вывода имени файла (-h)
  const char *program_name;  ///< Имя программы (для вывода ошибок)
  int binary_file;  ///< Флаг бинарного файла
} GrepOptions;

/**
 * @brief Обрабатывает аргументы командной строки
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 * @param opts Указатель на структуру параметров
 * @return Код ошибки (ErrorCode)
 */
static ErrorCode process_arguments(int argc, char **argv, GrepOptions *opts);

/**
 * @brief Обрабатывает флаги через geptopt
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 * @param opts Указатель на структуру параметров
 * @return Код ошибки (ErrorCode)
 */
static ErrorCode process_flags(int argc, char **argv, GrepOptions *opts);

/**
 * @brief Обрабатывает флаг -e (добавление шаблона)
 * @param opts Указатель на структуру параметров
 * @param arg Значение флага
 * @return Код ошибки (ErrorCode)
 */
static ErrorCode handle_flag_e(GrepOptions *opts, const char *arg);

/**
 * @brief Компилирует регулярные выражения из шаблонов
 * @param opts Указатель на структуру параметров
 * @return Код ошибки (ErrorCode)
 */
static ErrorCode compile_patterns(GrepOptions *opts);

/**
 * @brief Читает шаблоны из файла (флаг -f)
 * @param opts Указатель на структуру параметров
 * @param filename Имя файла с шаблонами
 * @return Код ошибки (ErrorCode)
 */
static ErrorCode read_patterns_from_file(GrepOptions *opts,
                                         const char *filename);

/**
 * @brief Освобождает выделенные ресурсы
 * @param opts Указатель на структуру параметров
 */
static void cleanup_resources(GrepOptions *opts);

/**
 * @brief Обрабатывает файл или стандартный ввод
 * @param filename Имя файла или "(standard input)"
 * @param opts Указатель на структуру параметров
 */
static void process_file(const char *filename, GrepOptions *opts);

/**
 * @brief Выводит префикс строки (имя файла/номер строки)
 * @param filename Имя файла
 * @param line_num Номер строки
 * @param opts Указатель на структуру параметров
 */
static void handle_match_output(const char *filename, int line_num,
                                const GrepOptions *opts);

/**
 * @brief Обрабатывает строку и проверяет на совпадения
 * @param buffer Строка для обработки
 * @param line_num Номер строки
 * @param opts Указатель на структуру параметров
 * @param filename Имя файла
 * @param match_count Счетчик совпадений (обновляется)
 */
static void process_line(const char *buffer, int line_num, GrepOptions *opts,
                         const char *filename, int *match_count);

/**
 * @brief Выводит итоговый счетчик совпадений
 * @param match_count Количество совпадений
 * @param opts Указатель на структуру параметров
 * @param filename Имя файла
 */
static void print_final_count(int match_count, const GrepOptions *opts,
                              const char *filename);

/**
 * @brief Основная функция поиска в файле
 * @param file Указатель на файл
 * @param opts Указатель на структуру параметров
 * @param filename Имя файла
 */
static void search(FILE *file, GrepOptions *opts, const char *filename);

/**
 * @brief Проверяет файл на бинарное содержимое
 * @param fp Указатель на файл
 * @param opts Указатель на структуру параметров
 * @param filename Имя файла
 */
static void processing_binary(FILE *fp, GrepOptions *opts,
                              const char *filename);

/**
 * @brief Обрабатывает совпадения в строке для флага -o
 * @param buffer Строка для обработки
 * @param opts Указатель на структуру параметров
 * @param line_num Номер строки
 * @param filename Имя файла
 * @return Указатель на обработанную строку
 */
static char *process_matches(const char *buffer, GrepOptions *opts,
                             int line_num, const char *filename);

/**
 * @brief Выводит строку без модификаций
 * @param buffer Строка для вывода
 */
static void print_plain_line(const char *buffer);

/**
 * @brief Устанавливает флаг бинарного файла
 * @param opts Указатель на структуру параметров
 * @param value Значение флага
 */
void set_grep_binary_flag(void *opts, int value);

#endif  // S21_GREP_H