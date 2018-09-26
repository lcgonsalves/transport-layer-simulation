all : transportlayersim

transportlayersim : project.o project2.h
	gcc -o transportlayersim project.o -Wall -Werror -lpthread

project.o : project2.c project2.h student2.c
	gcc -c project2.c project2.h student2.c -Wall -Werror

clean : 
	rm transportlayersim *.o