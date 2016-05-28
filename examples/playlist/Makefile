SOURCES = playlist.ml
RESULT = playlist
LIBS = bigarray gstreamer
INCDIRS = ../../src
THREADS = yes

all: dnc

test: all
	./playlist test.mp3 test.mp3 test.mp3

include OCamlMakefile
