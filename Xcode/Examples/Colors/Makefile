PRIMESDK:=/Applications/HP/PrimeSDK


ifndef SOURCES
	SOURCES := src
endif

ifndef NAME
	NAME := PROJECT
endif

all:
	mkdir -p build
	ppl+ $(SOURCES)/main -o build/$(NAME).prgm -I$(PRIMESDK)/usr/include/ -L$(PRIMESDK)/usr/lib/
min:
	pplmin build/$(NAME).prgm
clean:
	rmdir -r build
