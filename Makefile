# CS3350 Lab 1 - Waterfall Demo

LFLAGS = -lrt -lX11 -lGLU -lGL -lm #-lXrandr

all:  waterfall 

waterfall: waterfall.cpp
	g++ waterfall.cpp libggfonts.a timers.cpp -Wall -Wextra $(LFLAGS) -o waterfall

clean:
	rm -f waterfall 
	rm -f *.o

# g++ waterfall.cpp -Wall -owaterfall -lX11 -lGL -lGLU -lm
