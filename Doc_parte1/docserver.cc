#include "tools.h"


int main(int argc, char *argv[]) {
    // parse_args(): Procesar los argumentos de la línea de comandos
    auto result = parse_args(argc, argv);
    // Error fatal if error al procesar los argumentos
    if (!result) {
        switch (result.error()) {
            case parse_args_errors::missing_argument:
                std::cerr << "Error: missing argument\n";
                break;
            case parse_args_errors::unknown_option:
                std::cerr << "Error: unknown option\n";
                break;
            // Add other parse_args_errors cases here if needed
        }
        return EXIT_FAILURE;
    }

    // If las opciones contienen -h o --help Mostrar ayuda
    if (result->show_help) {
        show_help();
        return EXIT_SUCCESS;
    }

    // Error fatal if no se ha indicado un nombre de archivo en los argumentos
    if (result->additional_args.empty()) {
        std::cerr << "Error: missing filename\n";
        return EXIT_FAILURE;
    }

    // read_all(): Leer archivo en memoria
    auto content = read_all(result->additional_args[0], result->verbose);
    // Handle errores de permisos o archivo no encontrado
    if (!content) {
        int error_code = content.error();
        std::cerr << "Error: failed to read file '" << result->additional_args[0]
                  << "' (errno " << error_code << ": " << std::strerror(error_code) << ")\n";

        // Show appropriate error message using send_response()
        if (error_code == EACCES) {
            send_response("403 Forbidden", false);
        } else if (error_code == ENOENT) {
            send_response("404 Not Found", false);
        } else {
            send_response(std::format("500 Internal Server Error (errno: {})\n", error_code), false);
        }
        return EXIT_FAILURE;
    }

    // Caso de éxito
    SafeMap map(content.value());
    auto view = map.view();
    size_t size = view.size();

    std::ostringstream oss;
    oss << "Content-Length: " << size << '\n';
    std::string header = oss.str();
    // send_response(): Mostrar el contenido del archivo en la salida estándar
    send_response(header, result->show_size, view);

    return EXIT_SUCCESS;
}
