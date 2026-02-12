#ifndef IS_BINARY_FILE_H
#define IS_BINARY_FILE_H

#include <stdio.h>
#include <string.h>

#include "error.h"

typedef void (*set_binary_flag)(void *, int);

/**
 * @brief Проверка является ли файл бинарным
 * @param buf указатель на буффер
 * @param size размер буффера
 * @param fp указатель на файл
 * @param opts структура настроек
 * @param set_flag указатель на функцию установки флага (Callback)
 * @param filename имя файла
 * @param program_name имя программы
 */
void is_binary_file(char *buf, size_t size, FILE *fp, void *opts,
                    set_binary_flag set_flag, const char *filename,
                    const char *program_name);
#endif