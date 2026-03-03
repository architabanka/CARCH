CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2

OBJS = main.o archive.o bitio.o huffman.o
TARGET = carch

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
