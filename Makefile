

default: all

clean:
	rm *.exe
	
all: controller

controller: controller.c IO.c
	gcc -o controller controller.c  -llibmodbus -Wall -L /usr/local/lib

