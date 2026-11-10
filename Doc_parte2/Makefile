# Makefile para compilar con g++ y las opciones dadas (sin fsanitize)

# Nombre del ejecutable
EXEC = docserver

# Archivos fuente
SRC = *.cc

# Opciones de compilación
CXXFLAGS = -std=c++23 -Wall -Wextra -Werror -Wpedantic \
           -Wshadow -Wnon-virtual-dtor -Wold-style-cast \
           -Wcast-align -Wunused -Woverloaded-virtual \
           -Wconversion -Wsign-conversion -Wnull-dereference \
           -Wdouble-promotion -Wformat=2 -Wmisleading-indentation \
           -Wduplicated-cond -Wduplicated-branches -Wlogical-op \
           -Wuseless-cast

# Regla principal para construir el ejecutable
all: $(EXEC)

# Regla para compilar el ejecutable utilizando g++
$(EXEC): $(SRC)
	g++-14 $(CXXFLAGS) -o $(EXEC) $(SRC)

# Limpiar archivos generados
clean:
	rm -f $(EXEC)

# Regla para forzar la recompilación
rebuild: clean all
