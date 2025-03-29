// Copyright (c) 2025 mahd
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "./flag.h"
// Funktionsprototypen
void handle_exit(void);
void handle_help(void);
void handle_hello(void);

struct cmd_entry
{
  const char *name;
  void (*handler)(void);
  const char *description;
} commands[] = {
    {"exit", handle_exit, "Exit the shell"},
    {"quit", handle_exit, "Exit the shell"},
    {"help", handle_help, "Show this help message"},
    {"hello", handle_hello, "Print 'Hello, World!'"},
    {NULL, NULL, NULL} // Terminator
};

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
  puts("Available commands:");
  for (struct cmd_entry *cmd = commands; cmd->name ; cmd++)
  {
    printf("%s\t%s\n", cmd->name, cmd->description);
  }
}
static bool quit = false;
void process_input(const char *line)
{
  if (!line || !*line)
    return;

  add_history(line);

  // 1. Zuerst interne Kommandos prüfen
  for (struct cmd_entry *cmd = commands; cmd->name; cmd++)
  {
    if (strcmp(line, cmd->name) == 0)
    {
      cmd->handler();
      return;
    }
  }

  // 2. Für externe Kommandos

    printf("Unknown command: %s\n", line);
}

void usage(FILE *stream)
{
  fprintf(stream, "Usage: ./mysh [OPTIONS] [--] [INPUT FILE]\n");
  fprintf(stream, "OPTIONS:\n");
  flag_print_options(stream);
}
void versionp(FILE *stream)
{
  fprintf(stream, "mysh version 0.1.0-alpha\n");
}

int main(int argc, char *argv[]) {
  bool *help = flag_bool("-help", false, "Print this help");
  bool *help2 = flag_bool("h", false, "Print this help");
  bool *version = flag_bool("-version", false, "Print version");
  bool *version2 = flag_bool("v", false, "Print version");

  if (!flag_parse(argc, argv))
  {
    usage(stderr);
    flag_print_error(stderr);
    exit(1);
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
    while (!quit)
    {
      char *line = readline("mysh> ");
      if (!line)
        handle_exit();
      process_input(line);
      free(line);
    }

    handle_exit();
  }


}