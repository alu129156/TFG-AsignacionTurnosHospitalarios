CXX = g++
CXXFLAGS = -std=c++17 -Iinclude
SRC = ./src
BIN = ./bin

COMMON = $(SRC)/main_utils_v1.cpp \
         $(SRC)/empleado_v1.cpp \
         $(SRC)/solucion_v1.cpp \
         $(SRC)/general_utils_v1.cpp \
         $(SRC)/nodoAstar_v1.cpp

TARGETS = branchAndBound bruteForce heuristic_Astar heuristic_Astar_2 heuristic_Astar_3 heuristic_piston heuristic_tabuSearch

all: $(TARGETS)

$(BIN):
	powershell -Command "if (!(Test-Path -Path '$(BIN)')) { New-Item -ItemType Directory -Path '$(BIN)' }"

branchAndBound: $(COMMON) branchAndBound.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

bruteForce: $(COMMON) bruteForce.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

heuristic_Astar: $(COMMON) heuristic_Astar.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

heuristic_Astar_2: $(COMMON) heuristic_Astar_2.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

heuristic_Astar_3: $(COMMON) heuristic_Astar_3.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

heuristic_piston: $(COMMON) heuristic_piston.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

heuristic_tabuSearch: $(COMMON) heuristic_tabuSearch.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $^ -o $(BIN)/$@

clean:
	powershell -Command "if (Test-Path -Path '$(BIN)') { Remove-Item -Recurse -Force '$(BIN)' }"

.PHONY: all clean $(TARGETS)
