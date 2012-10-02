SOURCES = caps.ml
RESULT = caps
LIBS = bigarray gstreamer
INCDIRS = ../../src
THREADS = yes

all: dnc

test: all
	./caps test.mp3

include OCamlMakefile
