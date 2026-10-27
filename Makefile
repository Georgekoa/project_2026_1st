CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude $(shell pkg-config --cflags opencv4)
LDLIBS = $(shell pkg-config --libs opencv4)

TARGET = search
SOURCES = src/main.cpp src/functions.cpp src/kmeans.cpp src/algorithms.cpp src/evaluations.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET) src/search

.PHONY: all clean