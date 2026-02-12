#ifndef ERROR_CODES_H
#define ERROR_CODES_H

/**
 * @brief Коды возврата
 */
typedef enum {
  SUCCESS,        ///< Успешное выполнение
  PARSE_FAILURE,  ///< Ошибка разбора аргументов
  FGETS_ERROR,    ///< Ошибка чтения
  FILE_ERROR,     ///< Ошибка работы с файлом
  MEMORY_ERROR,   ///< Ошибка выделения памяти
  REGEX_ERROR  ///< Ошибка компиляции регулярного выражения
} ErrorCode;

#endif  // ERROR_CODES_H