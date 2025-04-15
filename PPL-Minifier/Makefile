NAME := pplmin
PRIMESDK := /Applications/HP/PrimeSDK
BUILD := Package\ Installer/package-root/Applications/HP/PrimeSDK/bin

all:
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp -o $(BUILD)/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	
install:
	cp $(BUILD)/$(NAME) $(PRIMESDK)/bin/$(NAME)
	
clean:
	rm -rf $(BUILD)/*
