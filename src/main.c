// Copyright (c) 2025 mahd
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#define NOB_IMPLEMENTATION
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"
#include "config.h"
#define ALEXER_IMPLEMENTATION
#include "alexer.h"
// Funktionsprototypen



typedef enum
{
  PUNCT_PLUS,
  PUNCT_MINUS,
  PUNCT_MULT,
  PUNCT_DEVIDE,
  PUNCT_MOD,
  PUNCT_EQUALS,
  PUNCT_OPAREN,
  PUNCT_CPAREN,
  PUNCT_OCURLY,
  PUNCT_CCURLY,
  PUNCT_SEMICOLON,
  COUNT_PUNCTS,
} Punct_Index;

static_assert(COUNT_PUNCTS == 11, "Amount of puncts have changed");
const char *puncts[COUNT_PUNCTS] = {
    [PUNCT_PLUS] = "+",
    [PUNCT_MINUS] = "-",
    [PUNCT_MULT] = "*",
    [PUNCT_DEVIDE] = "/",
    [PUNCT_MOD] = "%",
    [PUNCT_OPAREN] = "(",
    [PUNCT_CPAREN] = ")",
    [PUNCT_OCURLY] = "{",
    [PUNCT_CCURLY] = "}",
    [PUNCT_EQUALS] = "==",
    [PUNCT_SEMICOLON] = ";",
};

typedef enum
{
  KEYWORD_IF,
  KEYWORD_RETURN,
  KEYWORD_EXIT,
  KEYWORD_QUIT,
  KEYWORD_CD,
  KEYWORD_HELP,
  KEYWORD_HELLO,
  KEYWORD_ALIAS,
  KEYWORD_UNALIAS,
  KEYWORD_ECHO,
  KEYWORD_EXPORT,
  KEYWORD_UNSET,
  KEYWORD_SOURCE,
  KEYWORD_PRINT,
  KEYWORD_PRINTF,
  KEYWORD_PRINTLN,
  KEYWORD_PRINTFN,
  COUNT_KEYWORDS,
} Keyword_Index;

static_assert(COUNT_KEYWORDS == 17, "Amount of keywords have changed");
const char *keywords[COUNT_KEYWORDS] = {
    [KEYWORD_IF] = "if",
    [KEYWORD_RETURN] = "return",
    [KEYWORD_EXIT] = "exit",
    [KEYWORD_QUIT] = "quit",
    [KEYWORD_CD] = "cd",
    [KEYWORD_HELP] = "help",
    [KEYWORD_HELLO] = "hello",
    [KEYWORD_ALIAS] = "alias",
    [KEYWORD_UNALIAS] = "unalias",
    [KEYWORD_ECHO] = "echo",
    [KEYWORD_EXPORT] = "export",
    [KEYWORD_UNSET] = "unset",
    [KEYWORD_SOURCE] = "source",
    [KEYWORD_PRINT] = "print",
    [KEYWORD_PRINTLN] = "println",
};

const char *sl_comments[] = {
    "//",
    "#",
};

Alexer_ML_Comments ml_comments[] = {
    {"/*", "*/"},
};

typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} Cmd_external_args;

void handle_exit(void)
{
  printf("Exiting...\n");
  exit(0);
}

void handle_hello(void)
{
  printf("Hello, World!\n");
}

void handle_help(void)
{
    printf("Available commands:\n");
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i) {
        printf("- %s\n", keywords[i]);
    }
}
void handle_cd(char *args)
{
  static char previous_dir[PATH_MAX] = "";

  if (!args || strcmp(args, "~") == 0)
  {
    // Home-Verzeichnis
    char *home = getenv("HOME");
    if (!home)
    {
      fprintf(stderr, "cd: HOME not set\n");
      return;
    }
    args = home;
  }
  else if (strcmp(args, "-") == 0)
  {
    // Vorheriges Verzeichnis
    if (!previous_dir[0])
    {
      fprintf(stderr, "cd: no previous directory\n");
      return;
    }
    args = previous_dir;
  }

  // Aktuelles Verzeichnis speichern
  char cwd[PATH_MAX];
  if (!getcwd(cwd, sizeof(cwd)))
  {
    perror("cd");
    return;
  }

  if (chdir(args) != 0)
  {
    perror("cd");
    return;
  }

  // Vorheriges Verzeichnis aktualisieren
  strcpy(previous_dir, cwd);
}

static bool quit = false;
void process_input(const char *line, const char *file_path)
{
  if (!line || !*line)
    return;

  add_history(line);

  Alexer l = alexer_create(file_path, line, strlen(line));
  l.puncts = puncts;
  l.puncts_count = ALEXER_ARRAY_LEN(puncts);
  l.keywords = keywords;
  l.keywords_count = ALEXER_ARRAY_LEN(keywords);
  l.sl_comments = sl_comments;
  l.sl_comments_count = ALEXER_ARRAY_LEN(sl_comments);
  l.ml_comments = ml_comments;
  l.ml_comments_count = ALEXER_ARRAY_LEN(ml_comments);
  Alexer_Token t;
  while (alexer_get_token(&l, &t))
  {
    switch (ALEXER_KIND(t.id))
    {
    case ALEXER_KEYWORD:
    {
      switch (ALEXER_INDEX(t.id))
      {
      case KEYWORD_EXIT:
      case KEYWORD_QUIT:
        handle_exit();
        break;

      case KEYWORD_HELP:
        handle_help();
        break;

      case KEYWORD_HELLO:
        handle_hello();
        break;

      case KEYWORD_CD:
        if (alexer_get_token(&l, &t))
        {
          // Prüfen, ob das nächste Token ein String oder Symbol ist
          if (ALEXER_KIND(t.id) == ALEXER_STRING || ALEXER_KIND(t.id) == ALEXER_SYMBOL)
          {
            char *args = nob_temp_sprintf("%.*s", (int)(t.end - t.begin), t.begin);
            handle_cd(args);
          }
          else
          {
            fprintf(stderr, "cd: expected string argument\n");
          }
        }
        else
        {
          handle_cd(NULL);
        }
        break;
        break;
      case KEYWORD_ALIAS:
        break;
      case KEYWORD_UNALIAS:
        break;
      case KEYWORD_ECHO:
      case KEYWORD_PRINT:
        if (alexer_get_token(&l, &t))
        {
          // Prüfen, ob das nächste Token ein String oder Symbol ist
          if (ALEXER_KIND(t.id) == ALEXER_STRING || ALEXER_KIND(t.id) == ALEXER_SYMBOL)
          {
            printf("%.*s", (int)(t.end - t.begin), t.begin);
          }
          else
          {
            fprintf(stderr, "echo/print: expected string argument\n");
          }
        }
        else
        {
          fprintf(stderr, "echo/print: missing argument\n");
        }
        break;

      case KEYWORD_PRINTLN:
        if (alexer_get_token(&l, &t))
        {
          if (ALEXER_KIND(t.id) == ALEXER_STRING || ALEXER_KIND(t.id) == ALEXER_SYMBOL)
          {
            printf("%.*s\n", (int)(t.end - t.begin), t.begin);
          }
          else
          {
            fprintf(stderr, "println: expected string argument\n");
          }
        }
        else
        {
          fprintf(stderr, "println: missing argument\n");
        }
        break;

      default:
        nob_log(NOB_INFO, "Unknown keyword: %.*s\n", (int)(t.end - t.begin), t.begin);
        break;
      }
      break;
    }
    default:
      // 2. Für externe Kommandos
      Nob_Cmd cmd_extern = {0};                // Command extern (dynamic Array)
      Cmd_external_args cmd_extern_args = {0}; // Command extern args (dynamic Array)
      char *temp = strdup(line);               // copy of the line
      char *token = strtok(temp, " ");
      while (token != NULL)
      {
        // Append each token to the command
        nob_da_append(&cmd_extern_args, strdup(token));
        token = strtok(NULL, " ");
      }
      free(temp); // Free the copy of the line
      nob_da_foreach(const char **, token_ptr, &cmd_extern_args)
      {
        nob_cmd_append(&cmd_extern, *token_ptr); // Dereferenzieren des char**
      }
      nob_da_free(cmd_extern_args); // Free the command arguments
      if (!nob_cmd_run_sync_and_reset(&cmd_extern))
        printf("Unknown command: %s\n", line);
      break;
      printf("Unknown command: %.*s\n", (int)(t.end - t.begin), t.begin);
      break;
    }
      return;
    }
  }

void usage(FILE *stream)
{
  fprintf(stream, "%s - %s\n", flag_program_name(), PROGRAMM_DESCRIPTION);
  fprintf(stream, "%s\n", PROGRAMM_LICENSE);
  fprintf(stream, "%s\n", PROGRAMM_COPYRIGHT);
  fprintf(stream, "Usage: ./%s [OPTIONS] [--] [INPUT FILE]\n", flag_program_name());
  fprintf(stream, "OPTIONS:\n");
  flag_print_options(stream);
}
void versionp(FILE *stream)
{
  fprintf(stream, "%s - %s\n", flag_program_name(), PROGRAMM_VERSION);
}

int main(int argc, char **argv) {
  char *help_discription = "Display this help message.";
  char *version_discription = "Print the version of mysh";
  bool *help = flag_bool("-help", false, help_discription);
  bool *help2 = flag_bool("h", false, help_discription);
  bool *version = flag_bool("-version", false, version_discription);
  bool *version2 = flag_bool("v", false, version_discription);
  Nob_String_Builder sb = {0};


  if (!flag_parse(argc, argv))
  {
    usage(stderr);
    flag_print_error(stderr);
    return 1;
  }
  if (*help || *help2)
  {
    usage(stdout);
    exit(0);
  }

  if (*version || *version2)
  {
    versionp(stdout);
    exit(0);
  }

  int rest_argc = flag_rest_argc();
  char **rest_argv = flag_rest_argv();
  NOB_UNUSED(rest_argv);



    if (rest_argc <= 0)
    {
      nob_sb_append_cstr(&sb, flag_program_name());
      nob_sb_append_cstr(&sb, "> ");
      while (!quit)
      {
        char *line = readline(sb.items);
        if (!line)
        {
          handle_exit();

        }
        process_input(line, NULL);
        free(line);
      }
      nob_sb_free(sb);
      handle_exit();
    }
    else
    {
      const char *file_path = rest_argv[0];


      if (!nob_read_entire_file(file_path, &sb))
      {
        nob_log(NOB_ERROR, "Fehler beim Lesen der Datei %s: %s\n", file_path, strerror(errno));
        nob_sb_free(sb); // Sicherstellen, dass der Speicher freigegeben wird
        return 1;        // Fehlercode zurückgeben
      }
      process_input(sb.items, file_path);
      nob_sb_free(sb);
      exit(0);
    }
  NOB_UNREACHABLE("main");
  return 0;
}