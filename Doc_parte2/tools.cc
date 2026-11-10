#include "tools.h"


void show_help() {
  std::println("\nUsage: docserver [options] filename");
  std::println("Options:");
  std::println("  -h, --help    Show this help message");
  std::println("  -v, --verbose Show verbose output");
  std::println("  -p, --port    Set the port to listen to\n");
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
    else if (*it == "-p" || *it == "--port") {
      if (++it == end) {
        return std::unexpected(parse_args_errors::missing_argument);
      }
      options.port = std::stoi(std::string(*it));
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

  return std::string_view(static_cast<char*>(mem), size_t(length));
}

/*
void send_response(std::string_view header, std::string_view body) {
  std::cout << '\n' << header << '\n';
  std::cout << body << '\n';
}
*/

void log_verbose(bool verbose, const std::string& message) {
  if(verbose)
  std::cerr << message << '\n';
}

//Novedades
std::string getenv(const std::string& name) {
  char* value = getenv(name.c_str()); 
  if (value) {
    return std::string(value); 
    }
  else {
    return std::string();
  }
}

std::expected<SafeFD, int> make_socket(uint16_t port, bool verbose) {
    log_verbose(verbose, "socket: trying to create socket");
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log_verbose(verbose, std::format("socket: failed to create socket, errno: {}, error: {}", errno, strerror(errno)));
        return std::unexpected(errno);
    }
    log_verbose(verbose, std::format("socket: descriptor created with fd {}", sockfd));

    // Configurar opciones del socket
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        log_verbose(verbose, "setsockopt: failed to set SO_REUSEADDR");
        close(sockfd);
        return std::unexpected(errno);
    }
    log_verbose(verbose, "setsockopt: SO_REUSEADDR set successfully");

    // Configurar dirección del servidor
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    // Intentar vincular el socket al puerto
    log_verbose(verbose, std::format("bind: trying to bind socket to port {}", port));
    int result = bind(sockfd, reinterpret_cast<struct sockaddr*>(&serv_addr), sizeof(serv_addr));
    if (result < 0) {
        log_verbose(verbose, std::format("bind: failed to bind socket to port {}, errno: {}, error: {}", port, errno, strerror(errno)));
        close(sockfd); // Cerrar el descriptor si `bind` falla
        return std::unexpected(errno);
    }
    log_verbose(verbose, std::format("bind: socket successfully bound to port {}", port));

    return SafeFD(sockfd);
}


int listen_connection(const SafeFD& socket, bool verbose) {
  // Intentar poner el socket en modo de escucha
  log_verbose(verbose, std::format("listen: trying to listen on socket"));
  int result = listen(socket.get(), 5);
  if (result < 0) {
    log_verbose(verbose, std::format("listen: failed to listen on socket"));
    return errno;
  }
  log_verbose(verbose, std::format("listen: socket is listening successfully"));
  return ESUCCESS;
}

std::expected<SafeFD, int> accept_connection(const SafeFD& socket, sockaddr_in& client_addr, bool verbose) {
  log_verbose(verbose, std::format("accept: trying to accept connection"));
  socklen_t length;
  int fd = accept(socket.get(), reinterpret_cast<struct sockaddr*>(&client_addr), &length);

  if (fd < 0) {
    log_verbose(verbose, std::format("accept: failed to accept connection"));
    return std::unexpected(errno);
  }

  log_verbose(verbose, std::format("accept: connection accepted successfully"));

   // Si la conexión fue aceptada correctamente, crear un SafeFD con el nuevo descriptor de archivo
  return SafeFD(fd);
}

int send_response(const SafeFD& socket, std::string_view header, bool verbose, std::string_view body) {
  // Enviar el encabezado
  ssize_t sent = send(socket.get(), header.data(), header.size(), 0);
  log_verbose(verbose, std::format("send: trying to send header"));
  if (sent < 0) {
    log_verbose(verbose, std::format("send: failed to send header"));
    std::cerr << "Error: failed to send header\n";
    return errno;
  }
  log_verbose(verbose, std::format("send: header sent successfully"));

  // Enviar el cuerpo
  body = static_cast<std::string> (body) + '\n';
  sent = send(socket.get(), body.data(), body.size(), 0);
  log_verbose(verbose, std::format("send: trying to send body"));
  if (sent < 0) {
    log_verbose(verbose, std::format("send: failed to send body"));
    std::cerr << "Error: failed to send body\n";
    return errno;
  }
  log_verbose(verbose, std::format("send: body sent successfully"));
  log_verbose(verbose, std::format("Response sent successfully\n"));

  return ESUCCESS;
}