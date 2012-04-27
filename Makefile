CFLAGS = -ggdb --std=c99 -I./include -Wall
CXXFLAGS = -ggdb -I./include -Wall
LIBS = -lwinmm
LDFLAGS = --shared -g

all: bin/hear-now.dll bin/test.exe

clean:
	rm -rf bin/*
	find src -name \*.o -exec rm '{}' ';'

.PHONY: all clean

bin/test.exe: src/main.o
	$(CC) -o $@ $^ -Lbin -lhear-now

bin/hear-now.dll: src/audio.o src/mixer.o src/win32/audio-win32.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)