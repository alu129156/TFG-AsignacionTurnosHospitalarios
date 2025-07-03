CXX = g++
CXXFLAGS = -std=c++17 -Iinclude
SRC = ./src
BIN = ./bin
COMMON_PARAMS = ../common-params/src

COMMON = $(COMMON_PARAMS)/json_parser.cpp \
		 $(SRC)/main_utils_v2.cpp \
         $(SRC)/empleado_v2.cpp \
         $(SRC)/solucion_v2.cpp \
         $(SRC)/general_utils_v2.cpp \
         $(SRC)/nodoAstar_v2.cpp \
		 $(SRC)/turnos_v2.cpp

TARGETS = heuristic_Astar_2 heuristic_tabuSearch

all: $(TARGETS)

$(BIN):
	powershell -Command "if (!(Test-Path -Path '$(BIN)')) { New-Item -ItemType Directory -Path '$(BIN)' }"

heuristic_Astar_3: $(COMMON) heuristic_Astar_3.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

heuristic_tabuSearch: $(COMMON) heuristic_tabuSearch.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

clean:
	powershell -Command "if (Test-Path -Path '$(BIN)') { Remove-Item -Recurse -Force '$(BIN)' }"

.PHONY: all clean $(TARGETS)
