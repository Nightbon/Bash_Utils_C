#include "s21_cat.h"

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  CatOptions opts = {0};
  ErrorCode status = SUCCESS;
  opts.program_name = basename(argv[0]);

  status = process_long_args(&argc, &argv, &opts);
  if (status == SUCCESS) {
    status = process_short_args(argc, argv, &opts);
  }

  if (status == SUCCESS) {
    status = process_files(argc, argv, &opts);
  }

  return status;
}

static ErrorCode process_files(int argc, char **argv, CatOptions *opts) {
  ErrorCode status = SUCCESS;
  for (int i = optind; i < argc && status == SUCCESS; i++) {
    status = process_file(argv[i], opts);
  }
  return (optind == argc) ? process_file("(standard input)", opts) : status;
}

static ErrorCode process_long_args(int *argc, char ***argv, CatOptions *opts) {
  ErrorCode status = SUCCESS;
  int i, j;
  for (i = 1, j = 1; i < *argc && status == SUCCESS; i++) {
    if (strncmp((*argv)[i], "--", 2) == 0) {
      const char *opt = (*argv)[i] + 2;
      if (strcmp(opt, "number-nonblank") == 0) {
        opts->number_nonblank = 1;
      } else if (strcmp(opt, "number") == 0) {
        opts->number_all = 1;
      } else if (strcmp(opt, "squeeze-blank") == 0) {
        opts->squeeze_blank = 1;
      } else {
        fprintf(stderr, "%s: unrecognized option '%s'\n", opts->program_name,
                (*argv)[i]);
        status = PARSE_FAILURE;
      }
    } else {
      (*argv)[j++] = (*argv)[i];
    }
  }
  *argc = j;
  (*argv)[j] = NULL;

  return status;
}

static ErrorCode process_short_args(int argc, char **argv, CatOptions *opts) {
  ErrorCode status = SUCCESS;
  int opt;
  while ((opt = getopt(argc, argv, "beEnstT")) != -1 && status == SUCCESS) {
    switch (opt) {
      case 'b': {
        opts->number_nonblank = 1;
        break;
      }
      case 'e': {
        opts->show_ends = 1;
        opts->enable_v = 1;
        break;
      }
      case 'E': {
        opts->show_ends = 1;
        break;
      }
      case 'n': {
        opts->number_all = 1;
        break;
      }
      case 's': {
        opts->squeeze_blank = 1;
        break;
      }
      case 't': {
        opts->show_tabs = 1;
        opts->enable_v = 1;
        break;
      }
      case 'T': {
        opts->show_tabs = 1;
        break;
      }
      default: {
        status = PARSE_FAILURE;
        break;
      }
    }
  }
  return status;
}

static ErrorCode process_file(const char *filename, CatOptions *opts) {
  ErrorCode status = SUCCESS;
  FILE *fp = NULL;

  if (strcmp(filename, "(standard input)") == 0) {
    fp = stdin;
  } else {
    fp = fopen(filename, "rb");
  }

  if (fp) {
    processing_binary(fp, opts, filename);
    status = cat_file(fp, opts);
    if (fp != stdin) fclose(fp);
  } else {
    print_error(opts->program_name, filename, "No such file or directory");
  }
  return status;
}

static void processing_binary(FILE *fp, CatOptions *opts,
                              const char *filename) {
  size_t size = 1024;
  char buf[size];
  is_binary_file(buf, size, fp, opts, set_cat_binary_flag, filename,
                 opts->program_name);

  if (opts->binary_file) {
    size_t bytes;
    while ((bytes = fread(buf, 1, sizeof(buf), fp)) > 0) {
      fwrite(buf, 1, bytes, stdout);
    }

    if (ferror(fp)) {
      print_error(opts->program_name, filename, "Error fread");
    }
  }
  return;
}

void set_cat_binary_flag(void *opts, int value) {
  ((CatOptions *)opts)->binary_file = value;
  return;
}

static void print_escaped(int c, const CatOptions *opts) {
  if (opts->show_tabs && c == '\t') {
    printf("^I");
  } else if (opts->show_ends && c == '\n') {
    printf("$\n");
  } else if (c == '\n') {
    putchar('\n');
  } else if (opts->enable_v && !isprint(c) && c != '\t') {
    handle_non_printable(c);
  } else {
    putchar(c);
  }
  return;
}

static void handle_non_printable(int c) {
  if (c == 127) {
    printf("^?");
  } else if (c < 32) {
    printf("^%c", c + 64);
  } else {
    putchar(c);
  }
  return;
}

static void process_line(CatOptions *opts, const char *line, size_t len) {
  const int is_empty = (len == 1 && line[0] == '\n');

  if (!(opts->squeeze_blank && is_empty && opts->prev_empty)) {
    handle_line_numbering(opts, is_empty);
    print_line_content(line, len, opts);

    opts->prev_empty = is_empty;
  }

  return;
}

static void handle_line_numbering(CatOptions *opts, int is_empty) {
  if (opts->new_line && should_number_line(opts, is_empty)) {
    printf("%6u\t", ++opts->line_number);
    opts->new_line = 0;
  }
}

static int should_number_line(const CatOptions *opts, int is_empty) {
  return (opts->number_nonblank && !is_empty) ||
         (opts->number_all && !opts->number_nonblank);
}

static void print_line_content(const char *line, size_t len, CatOptions *opts) {
  for (size_t i = 0; i < len; i++) {
    print_escaped(line[i], opts);
    opts->new_line = (line[i] == '\n');
  }
  return;
}

static ErrorCode cat_file(FILE *fp, CatOptions *opts) {
  ErrorCode status = SUCCESS;
  char buffer[MAX_LINE_LEN];
  opts->prev_empty = 0;
  opts->new_line = 1;
  size_t len = 0;
  int start_string = 0;
  int done_string = 0;

  while (fgets(buffer, sizeof(buffer), fp) && (len = strlen(buffer)) &&
         !done_string) {
    process_line(opts, buffer, len);

    if (buffer[len - 1] != '\n') {
      start_string = 1;
    }

    if (start_string && buffer[len - 1] == '\n') {
      done_string = 1;
    }
  }

  if (ferror(fp)) {
    perror("fgets");
    status = FGETS_ERROR;
  }

  return status;
}