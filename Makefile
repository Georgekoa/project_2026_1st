CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Iinclude $(shell pkg-config --cflags opencv4)
LDLIBS = $(shell pkg-config --libs opencv4)

TARGET = search
SOURCES = src/main.cpp src/functions.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET) src/search

.PHONY: all clean