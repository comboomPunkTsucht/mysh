/**
 * Copyright (c) 2025 mahd
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */
#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#define FLAG_IMPLEMENTATION
#include "./flag.h"

// Some folder paths that we use throughout the build process.
#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"
#define LIB_FOLDER "./lib"
#define PROGRAMM_NAME "mysh"


int main(int argc, char **argv)
{
  //copy argc and argv for flags
  int flag_argc = argc;
  char **flag_argv = argv;
  // This line enables the self-rebuilding. It detects when nob.c is updated and auto rebuilds it then
  // runs it again.
  NOB_GO_REBUILD_URSELF(argc, argv);

  char **target = flag_str("-target", NULL, "sets the target for the build");
  if (!flag_parse(flag_argc, flag_argv))
  {
    flag_print_error(stderr);
    exit(1);
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
  cmd_append(&cmd, "cc", "-Wall", "-Wextra", "-Wformat", "-I", LIB_FOLDER);
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