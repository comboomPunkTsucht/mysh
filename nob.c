/**
 * Copyright (c) 2025 mahd
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define IS_NOB_C
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "flag.h"
#include "config.h"

// Some folder paths that we use throughout the build process.
#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define LIB_FOLDER "./lib"
#define CONFIG_FOLDER "./config"

void usage(FILE *stream)
{
  fprintf(stream, "nob - %s - Public Domain - https://github.com/tsoding/nob.h\n", NOB_VERSION, NOB_COPYRIGHT);
  fprintf(stream, "%s\n", NOB_COPYRIGHT);
  fprintf(stream, "Usage: ./nob [OPTIONS] [--]\n");
  fprintf(stream, "OPTIONS:\n");
  flag_print_options(stream);
}

void versionp(FILE *stream)
{
  fprintf(stream, "nob.h %s\n", NOB_VERSION);
}

int main(int argc, char **argv)
{
  //copy argc and argv for flags
  int flag_argc = argc;
  char **flag_argv = argv;
  // This line enables the self-rebuilding. It detects when nob.c is updated and auto rebuilds it then
  // runs it again.
  NOB_GO_REBUILD_URSELF(argc, argv);

  char *help_discription = "Display this help message.";
  char *version_discription = "Print the version of nob.h";

  char **target = flag_str("-target", NULL, "sets the target for the build");
  bool *help = flag_bool("-help", false, help_discription);
  bool *help2 = flag_bool("h", false, help_discription);
  bool *version = flag_bool("-version", false, version_discription);
  bool *version2 = flag_bool("v", false, version_discription);
  if (!flag_parse(flag_argc, flag_argv))
  {
    usage(stdout);
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

  // It's better to keep all the building artifacts in a separate build folder. Let's create it if it
  // does not exist yet.
  //
  // Majority of the nob command return bool which indicates whether operation has failed or not (true -
  // success, false - failure). If the operation returned false you don't need to log anything, the
  // convention is usually that the function logs what happened to itself. Just do
  // `if (!nob_function()) return;`
  if (!mkdir_if_not_exists(BUILD_FOLDER))
    return 1;



  // The working horse of nob is the Nob_Cmd structure. It's a Dynamic Array of strings which represent
  // command line that you want to execute.
  Cmd cmd = {0};
  cmd_append(&cmd,
    "cc",
    "-Wall",
    "-Wextra",
    "-Wformat",
    "-Wincompatible-pointer-types",
    "-I", LIB_FOLDER,
    "-I", CONFIG_FOLDER
  );
  if (*target)
  {
    char target_flag[128];
    snprintf(target_flag, sizeof(target_flag), "--target=%s", *target);
    // If you want to use the target flag you need to add it to the command line.
    cmd_append(&cmd, target_flag);
  }
  cmd_append(&cmd, "-o", BUILD_FOLDER PROGRAMM_NAME, SRC_FOLDER "main.c", "-lreadline");
  // nob_cmd_run_sync_and_reset() resets the cmd for you automatically
  if (!cmd_run_sync_and_reset(&cmd)) return 1;

  return 0;
}