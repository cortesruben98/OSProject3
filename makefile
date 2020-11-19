project3: project3.o
	gcc -std=c99 project3.o -o project3

project3.o: project3.c
	gcc -std=c99 -c -g project3.c

clean:
	rm project3.o
	rm a.out
	rm project3
