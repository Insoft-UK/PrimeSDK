LIB:=~/Documents/HP\ Prime/SDK/Libraries/pplib


ifndef SOURCES
	SOURCES := src
endif

ifndef NAME
	NAME := PROJECT
endif

all:
	mkdir -p build
	p+ $(SOURCES)/main.pp -l $(LIB) -o build/$(NAME)
clean:
	rmdir -r build
