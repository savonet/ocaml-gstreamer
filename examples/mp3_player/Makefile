SOURCES = player.ml
RESULT = player
LIBS = bigarray gstreamer
INCDIRS = ../../src
THREADS = yes

all: dnc

test: all
	./player ../../../ocaml-mm/examples/test.mp3

include OCamlMakefile
