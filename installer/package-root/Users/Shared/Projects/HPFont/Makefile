PRIMESDK:=/Applications/HP/PrimeSDK


ifndef SOURCES
	SOURCES := src
endif

ifndef NAME
	NAME := PROJECT
endif

all:
	mkdir -p build
	p+ $(SOURCES)/main.pp -l $(PRIMESDK)/pplib -o build/$(NAME).hpprgm
min:
	pplmin build/$(NAME).hpprgm
clean:
	rmdir -r build
