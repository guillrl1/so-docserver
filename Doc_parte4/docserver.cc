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
    // if las opciones contienen -h o --help Mostrar ayuda
    if(result->show_help) {
      show_help();
      return EXIT_SUCCESS;
    }

    // make_socket(): Crear un socket en el puerto especificado
    auto socket = make_socket(uint16_t(result->port), result->verbose);
    // error-fatal if error al crear el socket
    if(!socket) {
      std::cerr << "Error: failed to create socket\n";
      return EXIT_FAILURE;
    }
    SafeFD socketFD = std::move(socket.value());

    // listen_connection(): Poner el socket en modo de escucha
    int listen_result = listen_connection(socketFD, result->verbose);
    // error-fatal if error al poner el socket en modo de escucha
    if (listen_result != ESUCCESS) {
      std::cerr << "Error: failed to listen on socket\n";
      return EXIT_FAILURE;
    }

    // loop
    while (true) {
    sockaddr_in client_addr;

    // Aceptar una conexión
    auto client = accept_connection(socketFD, client_addr, result->verbose);
    if (!client) {
        std::cerr << "Error: failed to accept connection\n";
        continue; // Intentar con el siguiente cliente
    }

    SafeFD client_socket = std::move(client.value());

    // Leer la petición del cliente
    auto request = receive_request(client_socket, 1024, result->verbose);
    if (!request) {
        // Caso de error al recibir la petición
        int error_code = request.error();
        if (error_code == EACCES) {
            send_response(client_socket, "HTTP/1.1 403 Forbidden\n\n", result->verbose, {});
        } else if (error_code == ENOENT) {
            send_response(client_socket, "HTTP/1.1 404 Not Found\n\n", result->verbose, {});
        } else {
            std::cerr << "Error al recibir la petición: errno = " << error_code << '\n';
            send_response(client_socket,
                          std::format("HTTP/1.1 500 Internal Server Error (errno: {})\n\n", error_code),
                          result->verbose, {});
        }
        continue; // Intentar con el siguiente cliente
    }

    // Procesar la petición recibida
    auto file_path_result = process_request(request.value());
    if (!file_path_result) {
        // Error en la petición (no es válida o método no soportado)
        send_response(client_socket, "HTTP/1.1 400 Bad Request\n\n", result->verbose, {});
        continue;
    }

    std::string file_path = file_path_result.value();

    // Leer el archivo solicitado en memoria
    auto content = read_all(file_path, result->verbose); // Asegúrate de tener implementada esta función
    if (!content) {
        // Caso de error al leer el archivo
        int error_code = content.error();
        if (error_code == EACCES) {
            send_response(client_socket, "HTTP/1.1 403 Forbidden\n\n", result->verbose, {});
        } else if (error_code == ENOENT) {
            send_response(client_socket, "HTTP/1.1 404 Not Found\n\n", result->verbose, {});
        } else {
            std::cerr << "Error al leer el archivo: errno = " << error_code << '\n';
            send_response(client_socket,
                          std::format("HTTP/1.1 500 Internal Server Error (errno: {})\n\n", error_code),
                          result->verbose, {});
        }
        continue;
    }

    // Construir y enviar la respuesta
    SafeMap map(content.value());
    auto view = map.view();
    size_t size = view.size();

    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\n"
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
        continue; // Intentar con el siguiente cliente
    }
  }
}
