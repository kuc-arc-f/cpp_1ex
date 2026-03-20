CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
#CXXFLAGS = -Wall -O2 -I./include
LIBS     = -lsqlite3
TARGET   = server

.PHONY: all clean install

all: $(TARGET)

$(TARGET): server.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

install: $(TARGET)
	install -m 755 $(TARGET) $(HOME)/.local/bin/

clean:
	rm -f $(TARGET)