#include "s21_grep.h"

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  GrepOptions opts = {0};
  ErrorCode status = SUCCESS;
  opts.program_name = basename(argv[0]);

  if ((status = process_arguments(argc, argv, &opts)) == SUCCESS &&
      (status = compile_patterns(&opts)) == SUCCESS) {
    if (optind == argc) {
      process_file("(standard input)", &opts);
    } else {
      opts.print_filename =
          ((argc - optind > 1 && !opts.print_without_filename) ||
           opts.files_with_matches);
      for (int i = optind; i < argc; i++) {
        process_file(strcmp(argv[i], "-") == 0 ? "(standard input)" : argv[i],
                     &opts);
      }
    }
  }

  cleanup_resources(&opts);
  return status;
}

static ErrorCode process_arguments(int argc, char **argv, GrepOptions *opts) {
  ErrorCode status = process_flags(argc, argv, opts);

  if (status == SUCCESS && opts->num_patterns == 0 && optind < argc) {
    status = handle_flag_e(opts, argv[optind++]);
  }

  if (opts->files_with_matches) {
    opts->output_the_matched = 0;
    opts->count_only = 0;
    opts->line_number = 0;
  }

  if (opts->count_only) {
    opts->line_number = 0;
  }

  return status;
}

static ErrorCode process_flags(int argc, char **argv, GrepOptions *opts) {
  int opt;
  ErrorCode status = SUCCESS;
  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1 &&
         status == SUCCESS) {
    switch (opt) {
      case 'e': {
        status = handle_flag_e(opts, optarg);
        break;
      }
      case 'i': {
        opts->ignore_case = 1;
        break;
      }
      case 'v': {
        opts->invert_match = 1;
        break;
      }
      case 'c': {
        opts->count_only = 1;
        break;
      }
      case 'l': {
        opts->files_with_matches = 1;
        break;
      }
      case 'n': {
        opts->line_number = 1;
        break;
      }
      case 'h': {
        opts->print_without_filename = 1;
        break;
      }
      case 's': {
        opts->suppress_error = 1;
        break;
      }
      case 'f': {
        status = read_patterns_from_file(opts, optarg);
        break;
      }
      case 'o': {
        opts->output_the_matched = 1;
        break;
      }
      default: {
        status = PARSE_FAILURE;
      }
    }
  }
  return status;
}

static void process_file(const char *filename, GrepOptions *opts) {
  FILE *fp = NULL;

  if (strcmp(filename, "(standard input)") == 0) {
    fp = stdin;
  } else {
    fp = fopen(filename, "rb");
  }

  if (fp) {
    if (fp != stdin) processing_binary(fp, opts, filename);

    if (!opts->binary_file) {
      search(fp, opts, filename);
    }

    if (fp != stdin) fclose(fp);
  } else {
    if (!opts->suppress_error) {
      print_error(opts->program_name, filename, "No such file or directory");
    }
  }
  return;
}

static void search(FILE *file, GrepOptions *opts, const char *filename) {
  char *buffer = NULL;
  size_t buffer_size = 0;
  ssize_t read;
  int match_count = 0;
  int line_num = 0;
  ErrorCode status = SUCCESS;

  while ((read = getline(&buffer, &buffer_size, file)) != -1 &&
         status == SUCCESS) {
    if (!ferror(file)) {
      line_num++;
      if (read > 0 && buffer[read - 1] == '\n') {
        buffer[read - 1] = '\0';
      }
      process_line(buffer, line_num, opts, filename, &match_count);
    } else {
      if (!opts->suppress_error)
        print_error(opts->program_name, filename, "Error reading file");
      status = FILE_ERROR;
    }
  }

  free(buffer);
  if (match_count && status == SUCCESS)
    print_final_count(match_count, opts, filename);
  return;
}

static void process_line(const char *buffer, int line_num, GrepOptions *opts,
                         const char *filename, int *match_count) {
  int found = 0;
  for (size_t i = 0; i < opts->num_regexes && !found; i++) {
    found = (regexec(&opts->regexes[i], buffer, 0, NULL, 0) == 0) ^
            opts->invert_match;
  }
  *match_count += found;

  if (found && !opts->count_only && !opts->files_with_matches) {
    if (opts->output_the_matched) {
      process_matches(buffer, opts, line_num, filename);
    } else {
      handle_match_output(filename, line_num, opts);
      print_plain_line(buffer);
    }
  }

  return;
}

static void handle_match_output(const char *filename, int line_num,
                                const GrepOptions *opts) {
  if (opts->print_filename) {
    printf("%s:", filename);
  }

  if (opts->line_number) {
    printf("%d:", line_num);
  }

  return;
}

static void print_final_count(int match_count, const GrepOptions *opts,
                              const char *filename) {
  if (!opts->count_only && !opts->files_with_matches) return;

  if (opts->print_filename) {
    printf("%s", filename);
  }

  if (opts->files_with_matches) {
    putchar('\n');
  } else {
    if (opts->print_filename) printf(":");
    printf("%d\n", match_count);
  }

  return;
}

static char *process_matches(const char *buffer, GrepOptions *opts,
                             int line_num, const char *filename) {
  for (size_t i = 0; i < opts->num_regexes; i++) {
    const char *ptr = buffer;
    regmatch_t match;
    while (regexec(&opts->regexes[i], ptr, 1, &match, 0) == 0 &&
           match.rm_so != match.rm_eo) {
      handle_match_output(filename, line_num, opts);
      printf("%.*s\n", (int)(match.rm_eo - match.rm_so), ptr + match.rm_so);
      ptr += match.rm_eo;
    }
  }
  return (char *)buffer;
}

static ErrorCode handle_flag_e(GrepOptions *opts, const char *arg) {
  ErrorCode status = SUCCESS;
  char **new_ptr = NULL;
  char *escaped = strdup(arg);
  if (!escaped) {
    status = MEMORY_ERROR;
  } else {
    new_ptr =
        realloc(opts->patterns, (opts->num_patterns + 1) * sizeof(char *));
    if (!new_ptr) {
      free(escaped);
      status = MEMORY_ERROR;
    }
  }

  if (status == SUCCESS) {
    opts->patterns = new_ptr;
    opts->patterns[opts->num_patterns] = escaped;
    opts->num_patterns++;
  }

  return status;
}

static ErrorCode read_patterns_from_file(GrepOptions *opts,
                                         const char *filename) {
  ErrorCode status = SUCCESS;

  FILE *fp = fopen(filename, "r");
  if (!fp) {
    print_error(opts->program_name, filename, "No such file or directory");
    status = FILE_ERROR;
  } else {
    char *buffer = NULL;
    size_t len = 0;
    while (getline(&buffer, &len, fp) != -1 && status != FILE_ERROR) {
      if (!ferror(fp)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        status = handle_flag_e(opts, buffer);
      } else {
        print_error(opts->program_name, filename, "Error reading file");
        status = FILE_ERROR;
      }
    }
    free(buffer);
    fclose(fp);
  }

  return status;
}

static ErrorCode compile_patterns(GrepOptions *opts) {
  if (opts->num_patterns == 0) return PARSE_FAILURE;

  ErrorCode status = SUCCESS;
  int flags = (opts->ignore_case ? REG_ICASE : 0);
  if (!(opts->regexes = malloc(opts->num_patterns * sizeof(regex_t)))) {
    print_error(opts->program_name, "", "malloc");
    status = MEMORY_ERROR;
  } else {
    int rc = 0;
    for (size_t i = 0; i < opts->num_patterns && !rc; i++) {
      rc = regcomp(&opts->regexes[i], opts->patterns[i], flags);
      if (rc) {
        char errbuf[MAX_ERROR_MSG];
        regerror(rc, &opts->regexes[i], errbuf, sizeof(errbuf));
        print_error(opts->program_name, "", errbuf);
        status = REGEX_ERROR;
      } else {
        opts->num_regexes++;
      }
    }
  }

  return status;
}

static void processing_binary(FILE *fp, GrepOptions *opts,
                              const char *filename) {
  size_t size = 1024;
  char buf[size];
  is_binary_file(buf, size, fp, opts, set_grep_binary_flag, filename,
                 opts->program_name);

  if (opts->binary_file) {
    int found = 0;
    for (size_t i = 0; i < opts->num_regexes && !found; i++) {
      found = (regexec(&opts->regexes[i], buf, 0, NULL, 0) == 0) ^
              opts->invert_match;
    }
    if (found) print_error(opts->program_name, filename, "binary file matches");
  }

  return;
}

void set_grep_binary_flag(void *opts, int value) {
  ((GrepOptions *)opts)->binary_file = value;
}

static void cleanup_resources(GrepOptions *opts) {
  if (opts->patterns) {
    for (size_t i = 0; i < opts->num_patterns; i++) {
      free(opts->patterns[i]);
    }
    free(opts->patterns);
    opts->patterns = NULL;
  }
  if (opts->regexes) {
    for (size_t i = 0; i < opts->num_regexes; i++) {
      regfree(&opts->regexes[i]);
    }
    free(opts->regexes);
    opts->regexes = NULL;
    opts->num_regexes = 0;
  }

  return;
}

static void print_plain_line(const char *buffer) {
  printf("%s", buffer);
  if (buffer[strlen(buffer) - 1] != '\n') {
    putchar('\n');
  }

  return;
}