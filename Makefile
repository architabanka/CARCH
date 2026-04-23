CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2

LDFLAGS = -lcrypto

OBJS = main.o archive.o bitio.o huffman.o aes.o
TARGET = carch

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
