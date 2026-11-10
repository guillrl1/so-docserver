#include "tools.h"

int main(int argc, char *argv[]) {
    // parse_args(): Procesar los argumentos de la línea de comandos
    auto result = parse_args(argc, argv);
    // error-fatal if error al procesar los argumentos
    if(!result) {
      switch(result.error()) {
        case parse_args_errors::missing_argument:
          std::cerr << "Error: missing argument\n";
          break;
        case parse_args_errors::unknown_option:
          std::cerr << "Error: unknown option\n";
          break;
        // ...
      }
      return EXIT_FAILURE;
    }

    // if the options contain -h or --help Show help
    if(result->show_help) {
      show_help();
      return EXIT_SUCCESS;
    }

    // make_socket(): Create a socket on the specified port
    auto socket = make_socket(uint16_t(result->port), result->verbose);
    // error-fatal if error creating the socket
    if(!socket) {
      std::cerr << "Error: failed to create socket\n";
      return EXIT_FAILURE;
    }
    SafeFD socketFD = std::move(socket.value());

    // listen_connection(): Put the socket in listening mode
    int listen_result = listen_connection(socketFD, result->verbose);
    // error-fatal if error putting the socket in listening mode
    if (listen_result != ESUCCESS) {
      std::cerr << "Error: failed to listen on socket\n";
      return EXIT_FAILURE;
    }

    // loop
    while (true) {
    sockaddr_in client_addr;

    // Accept a connection
    auto client = accept_connection(socketFD, client_addr, result->verbose);
    if (!client) {
        std::cerr << "Error: failed to accept connection\n";
        continue; // Try with the next client
    }

    SafeFD client_socket = std::move(client.value());

    // Read the client's request
    auto request = receive_request(client_socket, 1024, result->verbose);
    if (!request) {
        // Case of error receiving the request
        int error_code = request.error();
        if (error_code == ECONNRESET) {
            std::cerr << "Error: connection reset by peer\n";
        } else {
            std::cerr << "Error: failed to receive request, errno: " << error_code << '\n';
            return EXIT_FAILURE;
        }
        continue; // Try with the next client
    }

    // Process the request
    auto file_path_result = process_request(request.value());
    if (!file_path_result) {
        // Error case processing the request (not valid or not supported)
        send_response(client_socket, " 400 Bad Request\n\n", result->verbose, {});
        continue;
    }

    std::string file_path = file_path_result.value();
    std::string full_path = result->basedir + file_path;
    // Read the file content
    auto content = read_all(full_path, result->verbose); 
    if (!content) {
        // Case of error reading the file
        int error_code = content.error();
        if (error_code == EACCES) {
            send_response(client_socket, " 403 Forbidden\n\n", result->verbose, {});
        } else if (error_code == ENOENT) {
            send_response(client_socket, " 404 Not Found\n\n", result->verbose, {});
        } else {
            std::cerr << "Error al leer el archivo: errno = " << error_code << '\n';
            send_response(client_socket,
                          std::format(" 500 Internal Server Error (errno: {})\n\n", error_code),
                          result->verbose, {});
        }
        continue;
    }

    // Build and send the response
    SafeMap map(content.value());
    auto view = map.view();
    size_t size = view.size();

    std::ostringstream oss;
    oss << " 200 OK\n"
        << "Content-Length: " << size << "\n"
        << "Content-Type: text/plain\n\n";

    std::string header = oss.str();

    int send_result = send_response(client_socket, header, result->verbose, view);
    if (send_result != ESUCCESS) {
        if (send_result == ECONNRESET) {
            std::cerr << "Error: connection reset by peer\n";
        } else {
            std::cerr << "Error: failed to send response, errno: " << send_result << '\n';
        }
        continue; // Try with the next client
    }
  }
}
