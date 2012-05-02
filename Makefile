ifeq ($(shell uname -s),Darwin)
    platform = $1
else # TODO(vogon): restrict this to Windows correctly
    platform = $2
endif

all: $(call platform,darwin,windows)

CFLAGS = --std=c99 -I./include -Isrc -Wall -g
LDFLAGS = --shared -g

OBJS_COMMON = src/audio.o src/mixer.o
OBJS_WIN32 = src/win32/audio-win32.o src/locks-pthread.o
OBJS_DARWIN = src/darwin/audio-darwin.o src/locks-pthread.o

windows: DEFINES = -DWINDOWS
windows: LIBS = -lwinmm -lpthread
windows: bin/hear-now.dll bin/test.exe

darwin: DEFINES = -DDARWIN
darwin: LIBS = -framework AudioToolbox -framework CoreFoundation
darwin: bin/libhear-now.dylib bin/test

clean:
	rm -f bin/*
	find src -name \*.o -exec rm '{}' ';'

.PHONY: all windows darwin clean

bin/test bin/test.exe : src/main.o
	$(CC) -o $@ $^ -Lbin -lhear-now

# TODO: can the below targets be combined?
bin/hear-now.dll: $(OBJS_COMMON) $(OBJS_WIN32)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

bin/libhear-now.dylib: $(OBJS_COMMON) $(OBJS_DARWIN)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.c.o:
	$(CC) -c $(CFLAGS) $(DEFINES) -o $@ $<
