#ifndef TOOLS_H
#define TOOLS_H

#define ESUCCESS 0

#include <expected>
#include <cerrno>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <print>
#include <sstream>
#include <unistd.h>
#include <sys/mman.h>
#include "SafeMap.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include "SafeFD.h"

enum class parse_args_errors {
  missing_argument,
  unknown_option,
  // ...
};
struct program_options {
  bool show_help = false;
  bool verbose = false;
  int port = 8080;
  // ...
  std::vector<std::string> additional_args;
};





void show_help();
std::expected<program_options, parse_args_errors> parse_args(int argc, char* argv[]);
std::expected<std::string_view, int> read_all(const std::string& path, bool verbose);
//void send_response(std::string_view header, std::string_view body = {});
void log_verbose(bool verbose, const std::string& message);
std::string getenv(const std::string& name);

std::expected<SafeFD, int> make_socket(uint16_t port, bool verbose);
int listen_connection(const SafeFD& socket, bool verbose);
std::expected<SafeFD, int> accept_connection(const SafeFD& socket, sockaddr_in& client_addr, bool verbose);
int send_response(const SafeFD& socket, std::string_view header, bool verbose, std::string_view body = {});
#endif // TOOLS_H
