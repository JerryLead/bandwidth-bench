CC=g++
CPPFLAGS = -march=native -O3 -std=c++11 -I./
LDFLAGS = -lpthread

DEPS = $(wildcard ./*.h)

all: main.cpp $(DEPS)
	$(CC) $(CPPFLAGS) -o test main.cpp