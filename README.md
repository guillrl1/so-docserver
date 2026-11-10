# DocServer · Sistemas Operativos

Servidor de documentos sobre sockets TCP en C++, desarrollado en cuatro partes para la asignatura Sistemas Operativos · Universidad de La Laguna · Grado en Ingeniería Informática.

## Contenido

| Carpeta | Descripción |
|---|---|
| `Doc_parte1` a `Doc_parte4` | Evolución del servidor; cada parte añade funcionalidad sobre la anterior |

Todas las partes incluyen utilidades de apoyo (`tools`), un mapa seguro (`SafeMap`) y, desde la segunda, un envoltorio de descriptores de fichero (`SafeFD`).

## Uso

```bash
make
./docserver [opciones] fichero
```

| Opción | Descripción |
|---|---|
| `-h`, `--help` | Muestra la ayuda |
| `-v`, `--verbose` | Salida detallada |
| `-p <puerto>`, `--port <puerto>` | Puerto de escucha |
| `-b <ruta>`, `--base <ruta>` | Directorio desde el que se sirven los ficheros |

## Autor

Guillermo López Concepción
