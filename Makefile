CFLAGS = --std=c99 -I./include
CXXFLAGS = -I./include
LIBS = -lwinmm
LDFLAGS = --shared

all: bin/hear-now.dll bin/test.exe

clean:
	rm -rf bin/*
	find src -name \*.o -exec rm '{}' ';'

.PHONY: all clean

bin/test.exe: src/main.o
	$(CC) -o $@ $^ -Lbin -lhear-now

bin/hear-now.dll: src/audio.o src/win32/audio-win32.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)