SOURCES = player.ml
RESULT = player
LIBS = unix gstreamer
CLIBS = camlidl
INCDIRS = ../../src

all: dnc

test: all
	./player ../../../ocaml-mm/examples/test.mp3

include OCamlMakefile