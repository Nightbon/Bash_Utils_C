#include "is_binary_file.h"

void is_binary_file(char *buf, size_t size, FILE *fp, void *opts,
                    set_binary_flag set_flag, const char *filename,
                    const char *program_name) {
  size_t bytes = fread(buf, 1, size, fp);
  if (ferror(fp)) {
    print_error(program_name, filename, "Error fread");
  } else {
    rewind(fp);
    int binary = 0;
    for (size_t i = 0; i < bytes && !binary; i++) {
      if (buf[i] == 0) binary = 1;
    }
    set_flag(opts, binary);
  }
  return;
}