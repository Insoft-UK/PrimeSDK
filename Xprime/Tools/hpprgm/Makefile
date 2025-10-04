NAME := hpprgm
BUILD := build

all:
	mkdir -p $(BUILD)
	g++ -arch x86_64 -arch arm64 -std=c++20 src/*.cpp -o $(BUILD)/$(NAME) -Os -fno-ident -fno-asynchronous-unwind-tables
	    
clean:
	rm -rf $(BUILD)/$(NAME)
	
install:
	cp $(BUILD)/$(NAME) ../../Developer/usr/bin/$(NAME)
	
uninstall:
	rm $(PRIMESDK)/bin/$(NAME)

