#include "error.h"

void print_error(const char *program_name, const char *filename,
                 const char *message) {
  if (strcmp(filename, "") == 0) {
    fprintf(stderr, "%s: %s\n", program_name, message);
  } else {
    fprintf(stderr, "%s: %s: %s\n", program_name, filename, message);
  }
  return;
}