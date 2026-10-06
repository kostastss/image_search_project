CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Iinc `pkg-config --cflags opencv4`
LDFLAGS = `pkg-config --libs opencv4`

SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build
BIN_DIR = bin

TARGET = $(BIN_DIR)/search-hp

# Ρητή δήλωση των αρχείων αντί για wildcard
SRCS = src/main.cpp src/utils.cpp src/dataset.cpp src/sift_extractor.cpp src/kmeans_vocab.cpp src/search_engine.cpp src/lsh.cpp src/hypercube.cpp scr/ivfflat.cpp src/ivfpq.cpp
OBJS = build/main.o build/utils.o build/dataset.o build/sift_extractor.o build/kmeans_vocab.o build/search_engine.o build/lsh.o build/hypercube.o build/ivfflat.o build/ivfpq.o

all: directories $(TARGET)

directories:
	mkdir -p build
	mkdir -p bin

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

build/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o$@

clean:
	rm -rf build bin

.PHONY: all clean directories