CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
LDFLAGS = -lcrypto

# OpenSSL support for macOS (Homebrew)
ifeq ($(shell uname), Darwin)
    OPENSSL_DIR = $(shell brew --prefix openssl@3 2>/dev/null || brew --prefix openssl 2>/dev/null)
    ifneq ($(OPENSSL_DIR),)
        CXXFLAGS += -I$(OPENSSL_DIR)/include
        LDFLAGS += -L$(OPENSSL_DIR)/lib
    endif
endif

OBJS = main.o archive.o bitio.o huffman.o aes.o
TARGET = carch

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
