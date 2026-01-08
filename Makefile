CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -I./include
LDFLAGS =

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INCLUDE_DIR = include

TARGET = $(BIN_DIR)/memsim

SOURCES = $(SRC_DIR)/main.cpp \
          $(SRC_DIR)/allocator/memory_block.cpp \
          $(SRC_DIR)/allocator/memory_allocator.cpp \
          $(SRC_DIR)/buddy/buddy_allocator.cpp \
          $(SRC_DIR)/cache/cache.cpp \
          $(SRC_DIR)/virtual_memory/virtual_memory.cpp

OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

all: $(TARGET)

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR)/allocator $(OBJ_DIR)/buddy $(OBJ_DIR)/cache $(OBJ_DIR)/virtual_memory:
	mkdir -p $@

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) $(OBJ_DIR)/allocator $(OBJ_DIR)/buddy $(OBJ_DIR)/cache $(OBJ_DIR)/virtual_memory
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Cleaned build artifacts"

clean-output:
	rm -rf output/*.txt
	@echo "Cleaned output files"

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@mkdir -p output
	@echo "Running memory allocator test..."
	@./$(TARGET) tests/workload_allocator.txt < tests/workload_allocator.txt
	@echo "Running cache test..."
	@./$(TARGET) tests/workload_cache.txt < tests/workload_cache.txt
	@echo "Running virtual memory test..."
	@./$(TARGET) tests/workload_vm.txt < tests/workload_vm.txt
	@echo "Running integrated system test..."
	@./$(TARGET) tests/workload_integrated.txt < tests/workload_integrated.txt
	@echo ""
	@echo "All tests complete! Output files generated in output/ directory:"
	@ls -lh output/*.txt

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/
	@echo "Installed to /usr/local/bin/"

uninstall:
	rm -f /usr/local/bin/memsim
	@echo "Uninstalled from /usr/local/bin/"

help:
	@echo "Memory Management Simulator - Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build the simulator (default)"
	@echo "  clean        - Remove build artifacts"
	@echo "  clean-output - Remove output files"
	@echo "  run          - Build and run the simulator"
	@echo "  test         - Run all test workloads and generate output files"
	@echo "  install      - Install to /usr/local/bin/"
	@echo "  uninstall    - Remove from /usr/local/bin/"
	@echo "  help         - Show this help message"

.PHONY: all clean clean-output run test install uninstall help
