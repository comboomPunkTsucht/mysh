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
void handle_exit(char **args);
void handle_help(char **args);
void handle_hello(char **args);
void handle_cd(char **args);

void handle_echo_print(char **args) { printf("%s", *args); }
void handle_println(char **args) { printf("%s\n", *args); }
void handle_printf(char **args) { printf("%s", *args); }
void handle_printfn(char **args) { printf("%s\n", *args); }


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
    [KEYWORD_PRINTF] = "printf",
    [KEYWORD_PRINTLN] = "println",
    [KEYWORD_PRINTFN] = "printfn",
};

const char *sl_comments[] = {
    "//",
    "#",
};

Alexer_ML_Comments ml_comments[] = {
    {"/*", "*/"},
};
struct cmd_entry
{
  const char *name;
  void  (*handler)(char **args);
  const char *description;
} commands[] = {
    {"exit", handle_exit, "Exit the shell"},
    {"quit", handle_exit, "Exit the shell"},
    {"cd", handle_cd, "Change directory"},
    {"help", handle_help, "Show this help message"},
    {"hello", handle_hello, "Print 'Hello, World!'"},
    {NULL, NULL, NULL} // Terminator
};

typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} Cmd_external_args;

void handle_exit(char **args)
{
  NOB_UNUSED(args);
  printf("Exiting...\n");
  exit(0);
}

void handle_hello(char **args)
{
  NOB_UNUSED(args);
  printf("Hello, World!\n");
}

void handle_help(char **args)
{
  NOB_UNUSED(args);
  puts("Available commands:");
  for (struct cmd_entry *cmd = commands; cmd->name ; cmd++)
  {
    printf("%s\t%s\n", cmd->name, cmd->description);
  }
}
void handle_cd(char **args)
{
  static char previous_dir[PATH_MAX] = "";

  if (!args[0] || strcmp(args[0], "~") == 0)
  {
    // Home-Verzeichnis
    char *home = getenv("HOME");
    if (!home)
    {
      fprintf(stderr, "cd: HOME not set\n");
      return;
    }
    args[0] = home;
  }
  else if (strcmp(args[0], "-") == 0)
  {
    // Vorheriges Verzeichnis
    if (!previous_dir[0])
    {
      fprintf(stderr, "cd: no previous directory\n");
      return;
    }
    args[0] = previous_dir;
  }

  // Aktuelles Verzeichnis speichern
  char cwd[PATH_MAX];
  if (!getcwd(cwd, sizeof(cwd)))
  {
    perror("cd");
    return;
  }

  if (chdir(args[0]) != 0)
  {
    perror("cd");
    return;
  }

  // Vorheriges Verzeichnis aktualisieren
  strcpy(previous_dir, cwd);
}
static bool quit = false;
void process_input(const char *line)
{
  if (!line || !*line)
    return;

  add_history(line);

  // 1. Zuerst interne Kommandos prüfen
  char *args_line[64] = {0};
  char *temp = strdup(line); // copy of the line
  char *token_line = strtok(temp, " ");
  int arg_count = 0;

  while (token_line != NULL && arg_count < 63)
  {
    args_line[arg_count++] = token_line;
    token_line = strtok(NULL, " ");
  }

  for (struct cmd_entry *cmd = commands; cmd->name; cmd++)
  {
    if (strcmp(args_line[0], cmd->name) == 0)
    {
      cmd->handler(&args_line[1]); // Argumente übergeben
      return;
    }
  }

  // 2. Für externe Kommandos
  Nob_Cmd cmd_extern = {0}; // Command extern (dynamic Array)
  Cmd_external_args cmd_extern_args = {0}; // Command extern args (dynamic Array)
  temp = strdup(line); // copy of the line
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
  if (!nob_cmd_run_sync_and_reset(&cmd_extern)) printf("Unknown command: %s\n", line);

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
          handle_exit(NULL);

        }
        process_input(line);
        free(line);
      }
      nob_sb_free(sb);
      handle_exit(NULL);
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
      printf("Dateiinhalt:\n%s\n", sb.items);
      nob_sb_free(sb);
      handle_exit(NULL);
    }
  NOB_UNREACHABLE("main");
  return 0;
}