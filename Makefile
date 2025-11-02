CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system
TARGET = snake
SOURCE = snake.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: clean

