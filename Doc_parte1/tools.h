#ifndef TOOLS_H
#define TOOLS_H

#include <expected>
#include <cerrno>
#include <vector>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <print>
#include <sstream>
#include <cstring> // For std::strerror
#include <unistd.h>
#include <sys/mman.h>
#include "SafeMap.h"

enum class parse_args_errors {
  missing_argument,
  unknown_option,
  // ...
};
struct program_options {
  bool show_help = false;
  bool verbose = false;
  bool show_size = true;
  // ...
  std::vector<std::string> additional_args;
};

void show_help();
std::expected<program_options, parse_args_errors> parse_args(int argc, char* argv[]);
std::expected<std::string_view, int> read_all(const std::string& path, bool verbose);
void send_response(std::string_view header, bool show, std::string_view body = {});
void log_verbose(bool verbose, const std::string& message);



#endif // TOOLS_H