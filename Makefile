CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system

all: snake tetris

snake: snake.cpp
	$(CXX) $(CXXFLAGS) -o snake snake.cpp $(LDFLAGS)

tetris: tetris.cpp
	$(CXX) $(CXXFLAGS) -o tetris tetris.cpp $(LDFLAGS)

clean:
	rm -f snake tetris

.PHONY: clean all snake tetris

