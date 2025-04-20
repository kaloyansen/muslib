CXX = g++
CXXFLAGS = -g -Wall
CXXFLAGS += -std=c++14
CXXFLAGS += -Wextra -pedantic -O2
CXXFLAGS += -I/usr/include/mysql
LIBS = -lsoci_core -lsoci_mysql -lmariadb

SRC = src/muslib.cpp
OBJ = $(patsubst src/%.cpp,bin/%.o,$(SRC)) 
BINARY = bin/muslib

all: $(BINARY)

$(BINARY): $(OBJ)
	$(CXX) $(OBJ) -o $(BINARY) $(LIBS)

bin/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ)

clobber: clean
	rm -rf $(BINARY)

run: $(BINARY)
	./$(BINARY)
