NAME := pplfont
PRIMESDK := /Applications/HP/PrimeSDK
BUILD := ../Package\ Installer/package-root/Applications/HP/PrimeSDK/bin
SRC := src

all:
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp ../PrimePlus/src/utf.cpp -o $(BUILD)/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	    
clean:
	rm -rf $(BUILD)/$(NAME)
	
install:
	cp $(BUILD)/$(NAME) $(PRIMESDK)/bin/$(NAME)
	
uninstall:
	rm $(PRIMESDK)/bin/$(NAME)

