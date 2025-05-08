PRIMESDK:=/Applications/HP/PrimeSDK


ifndef SOURCES
	SOURCES := src
endif

ifndef NAME
	NAME := PROJECT
endif

all:
	mkdir -p build
	ppl+ $(SOURCES)/main.pp -o build/$(NAME).hpprgm
min:
	pplmin build/$(NAME).hpprgm
clean:
	rmdir -r build
