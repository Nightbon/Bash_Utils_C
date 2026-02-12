#ifndef S21_CAT_H
#define S21_CAT_H

#include <ctype.h>
#include <libgen.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../common/error.h"
#include "../common/error_codes.h"
#include "../common/is_binary_file.h"

#define MAX_LINE_LEN 4096

typedef struct {
  int number_nonblank;       ///< -b, --number-nonblank
  int number_all;            ///< -n, --number
  int squeeze_blank;         ///< -s, --squeeze-blank
  int show_ends;             ///< -e/-E
  int show_tabs;             ///< -t/-T
  int enable_v;              ///< Активируется через -e/-t
  unsigned line_number;      ///< Текущий номер строки
  int prev_empty;            ///< Для сжатия пустых строк
  int new_line;              ///< Флаг начала новой строки
  const char *program_name;  ///< Имя программы
  int binary_file;           ///< Флаг бинарного файла
} CatOptions;

/**
 * @brief Разбор GNU аргументов командной строки
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 * @param opts Структура настроек
 * @return Код ошибки
 */
static ErrorCode process_long_args(int *argc, char ***argv, CatOptions *opts);

/**
 * @brief Разбор POSIX аргументов командной строки
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 * @param opts Структура настроек
 * @return Код ошибки
 */
static ErrorCode process_short_args(int argc, char **argv, CatOptions *opts);

/**
 * @brief Обработка всех файлов
 * @param argc Количество аргументов
 * @param argv Массив аргументов
 * @param opts Структура настроек
 * @return Код ошибки
 */
static ErrorCode process_files(int argc, char **argv, CatOptions *opts);

/**
 * @brief Обработка одного файла
 * @param filename Имя файла
 * @param opts Структура настроек
 * @return Код ошибки
 */
static ErrorCode process_file(const char *filename, CatOptions *opts);

/**
 * @brief Обработка бинарного файла
 * @param fp указатель на файл
 * @param opts Структура настроек
 * @param filename Имя файла
 */
static void processing_binary(FILE *fp, CatOptions *opts, const char *filename);

/**
 * @brief установка флага: бинарный файл (Callback)
 * @param opts структура настроек
 * @param value значение флага
 */
void set_cat_binary_flag(void *opts, int value);

/**
 * @brief Вывод символа с учетом экранирования
 * @param c Обрабатываемый символ
 * @param opts Структура настроек
 */
static void print_escaped(int c, const CatOptions *opts);

/**
 * @brief Обработка непечатаемых символов
 * @param c Обрабатываемый символ
 */
static void handle_non_printable(int c);

/**
 * @brief Обработка одной строки
 * @param opts Структура настроек
 * @param line Обрабатываемая строка
 * @param len Длина строки
 */
static void process_line(CatOptions *opts, const char *line, size_t len);

/**
 * @brief Управление нумерацией строк
 * @param opts Структура настроек
 * @param is_empty Флаг пустой строки
 */
static void handle_line_numbering(CatOptions *opts, int is_empty);

/**
 * @brief Определение необходимости нумерации
 * @param opts Структура настроек
 * @param is_empty Флаг пустой строки
 * @return 1(true) или 0(false)
 */
static int should_number_line(const CatOptions *opts, int is_empty);

/**
 * @brief Вывод содержимого строки
 * @param line Обрабатываемая строка
 * @param len Длина строки
 * @param opts Структура настроек
 */
static void print_line_content(const char *line, size_t len, CatOptions *opts);

/**
 * @brief Управляет циклом обработки строк
 * @param fp указатель на файл
 * @param opts Структура настроек
 * @return Код ошибки
 */
static ErrorCode cat_file(FILE *fp, CatOptions *opts);

#endif  // S21_CAT_H