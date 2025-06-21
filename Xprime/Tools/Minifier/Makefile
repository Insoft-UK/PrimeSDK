NAME := pplmin
BUILD := build

all:
	mkdir -p $(BUILD)
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp ../PrimePlus/src/utf.cpp -o $(BUILD)/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
install:
	cp $(BUILD)/$(NAME) ../../Developer/usr/bin/$(NAME)
	
clean:
	rm $(PRIMESDK)/bin/$(NAME)
