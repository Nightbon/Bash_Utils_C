#include <stdio.h>
#include <string.h>

#ifndef PRINT_ERROR_H
#define PRINT_ERROR_H

/**
 * @brief выводит сообщение об ошибке
 * @param program_name имя программы
 * @param filename имя файла
 * @param message сообщение об ошибке
 */
void print_error(const char *program_name, const char *filename,
                 const char *message);
#endif