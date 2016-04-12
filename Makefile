# cs335 lab1
# to compile your project, type make and press enter
CFLAGS = -I ./include
##LIB = ./libggfonts.so
LFLAGS = -lrt -lX11 -lGLU -lGL -lm #-lXrandr

all: hw1

hw1: hw1.cpp
	g++ $(CFLAGS) hw1.cpp libggfonts.a $(LFLAGS) -ohw1

clean:
	rm -f hw1
	rm -f *.o

