#include "tools.h"

void show_help() {
  std::println("\nUsage: docserver [options] filename");
  std::println("Options:");
  std::println("  -h, --help    Show this help message");
  std::println("  -v, --verbose Show verbose output");
  std::println("  -n            Do not show the size of the file");
}

std::expected<program_options, parse_args_errors> parse_args(int argc, char* argv[]) {
  std::vector<std::string_view> args(argv + 1, argv + argc);
  program_options options;

  for (auto it = args.begin(), end = args.end(); it != end; ++it) {
    if (*it == "-h" || *it == "--help") {
      options.show_help = true; 
    }
    else if (*it == "-v" || *it == "--verbose") {
      options.verbose = true;
    }
    else if (*it == "-n" ) {
      options.show_size = false;
    }
      // Procesar otras opciones...
    else if(!it->starts_with("-")) {
      options.additional_args.push_back(std::string(*it));
    }
    else {
    return std::unexpected(parse_args_errors::unknown_option);
    } 
  } 
  return options; 
}

std::expected<std::string_view, int> read_all(const std::string& path, bool verbose) {
  // Abrir archivo en modo lectura
  log_verbose(verbose, std::format("open: trying to open file \"{}\"", path));
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    log_verbose(verbose, std::format("open: failed to open file \"{}\"", path));
    return std::unexpected(errno);
  }
  log_verbose(verbose, std::format("open: file \"{}\" opened successfully", path));

  // Obtener longitud del archivo
  long long int length = lseek(fd, 0, SEEK_END);
  if (length < 0) {
    log_verbose(verbose, std::format("lseek: failed to get file size \"{}\"", path));
    close(fd);
    return std::unexpected(errno);
  }
  log_verbose(verbose, std::format("lseek: file size of \"{}\" is {}", path, length));

  log_verbose(verbose, std::format("mmap: trying to map file \"{}\"", path));
  void* mem = mmap(NULL, size_t(length), PROT_READ, MAP_PRIVATE, fd, 0);
  if(mem == MAP_FAILED) {
    log_verbose(verbose, std::format("mmap: failed to map file \"{}\"", path));
    return std::unexpected(errno);
  }
  log_verbose(verbose, std::format("mmap: file \"{}\" mapped successfully", path));
  close(fd);
  log_verbose(verbose, std::format("close: file \"{}\" closed successfully", path));
  return std::string_view(static_cast<char*>(mem), size_t(length));
}


void send_response(std::string_view header, bool show, std::string_view body) {
  if (show) {
  std::cout << '\n' << header << '\n';
  }
  std::cout << body << '\n';
}

void log_verbose(bool verbose, const std::string& message) {
  if(verbose)
  std::cerr << message << '\n';
}