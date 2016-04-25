SOURCES = duration.ml
RESULT = duration
LIBS = bigarray gstreamer
INCDIRS = ../../src
THREADS = yes

all: dnc

test: all
	./duration test.mp3

include OCamlMakefile
