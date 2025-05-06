CXX = g++
CXXFLAGS = -std=c++17 -Iinclude
SRC = ./src
BIN = ./bin

COMMON = $(SRC)/main_utils.cpp \
         $(SRC)/empleado.cpp \
         $(SRC)/solucion.cpp \
         $(SRC)/general_utils.cpp \
         $(SRC)/nodoAstar.cpp

TARGETS = branchAndBound_1 branchAndBound_2 bruteForce heuristic_Astar_3 heuristic_tabuSearch

all: $(TARGETS)

$(BIN):
	powershell -Command "if (!(Test-Path -Path '$(BIN)')) { New-Item -ItemType Directory -Path '$(BIN)' }"

branchAndBound_1: $(COMMON) branchAndBound_1.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

branchAndBound_2: $(COMMON) branchAndBound_2.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

bruteForce: $(COMMON) bruteForce.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

heuristic_Astar_3: $(COMMON) heuristic_Astar_3.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

heuristic_tabuSearch: $(COMMON) heuristic_tabuSearch.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

clean:
	powershell -Command "if (Test-Path -Path '$(BIN)') { Remove-Item -Recurse -Force '$(BIN)' }"

.PHONY: all clean $(TARGETS)
