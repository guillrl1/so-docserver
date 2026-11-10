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
    
    // error-fatal if NO se ha indicado un nombre de archivo en los argumentos
    if(result->additional_args.empty()) {
      std::cerr << "Error: missing filename\n";
      return EXIT_FAILURE;
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

      // accept_connection(): Aceptar una conexión
      auto client = accept_connection(socketFD, client_addr, result->verbose);
      // error-fatal if error al aceptar la conexión
      if (!client) {
        std::cerr << "Error: failed to accept connection\n";
        continue; // Seguimos con la siguiente conexión
      }

      SafeFD client_socket = std::move(client.value());

      // read_all(): Leer archivo en memoria
      auto content = read_all(result->additional_args[0], result->verbose);
      // if error de permisos o archivo no encontrado
      if(!content) {
        //Caso de error 
        // send_response(): Mostrar mensaje de error leve (ver Sección 1.1.2)
        int error_code = content.error();
        if (error_code == EACCES) {
          send_response(client_socket, "403 Forbidden", result->verbose, {});
        } 
        else if (error_code == ENOENT) {
          send_response(client_socket, "404 Not Found", result->verbose, {});
        } 
        else {
          send_response(client_socket, std::format("500 Internal Server Error (errno: {})\n", error_code), result->verbose, {});
        }
        return EXIT_SUCCESS;
      }
      else {
      //Caso de éxito
      SafeMap map(content.value());
      auto view = map.view();
      size_t size = view.size();

      std::ostringstream oss;
      oss << "Content-Length: " << size << '\n';
      std::string header = oss.str();

      // send_response(): Mostrar el contenido del archivo
      int send_result = send_response(client_socket, header, result->verbose, view);
      // error-fatal if error al enviar el contenido
      if (send_result != ESUCCESS) {
        if (send_result == ECONNRESET) {
          std::cerr << "Error: connection reset by peer\n";
        } 
        else {
          std::cerr << "Error: failed to send response, errno: ";
          break; //Error fatal
        }
      }
    }
  }
  return 0;
}
