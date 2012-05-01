CFLAGS = -ggdb --std=c99 -I./include -Isrc -Wall
CXXFLAGS = -ggdb -I./include -Isrc -Wall
WINDOWS_LIBS = -lwinmm -lpthread
WINDOWS_MACROS = -DWINDOWS
DARWIN_FRAMEWORKS = -framework AudioToolbox -framework CoreFoundation
DARWIN_MACROS = -DDARWIN
LDFLAGS = --shared -g

windows: bin/hear-now.dll bin/test.exe
darwin: bin/libhear-now.dylib bin/test

clean:
	rm -f bin/*
	find src -name \*.o -exec rm '{}' ';'

.PHONY: all clean

bin/test.exe: src/main.o
	$(CC) $(WINDOWS_MACROS) -o $@ $^ -Lbin -lhear-now

bin/test: src/main.o
	$(CC) $(DARWIN_MACROS) -o $@ $^ -Lbin -lhear-now

bin/hear-now.dll: src/audio.o src/mixer.o src/win32/audio-win32.o src/locks-pthread.o
	$(CC) $(WINDOWS_MACROS) $(LDFLAGS) -o $@ $^ $(WINDOWS_LIBS)

bin/libhear-now.dylib: src/audio.o src/mixer.o src/darwin/audio-darwin.o src/locks-pthread.o
	$(CC) $(DARWIN_MACROS) $(LDFLAGS) -o $@ $^ $(DARWIN_FRAMEWORKS)
