SOURCES = gstreamer_idl.idl gstreamer.ml
RESULT = gstreamer

CFLAGS = $(shell pkg-config --cflags gstreamer-0.10)
ACLIBS = $(shell pkg-config --libs gstreamer-0.10 | sed 's/-pthread//')
CLIBS = $(ACLIBS:-l%=%)

all: ncl

test: all
	ocamlopt -o test -I . unix.cmxa gstreamer.cmxa test.ml
	./test

include OCamlMakefile
