jack: jack.c project.c
	gcc -O3 -o jack jack.c project.c `pkg-config --cflags --libs jack` -lm

clean: 
	rm -f *.o

clean-all: clean
	rm -f *~ jack


