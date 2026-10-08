FATHOM = external/Fathom/src

main: main.cpp
	gcc -O2 -I$(FATHOM) -c $(FATHOM)/tbprobe.c -o tbprobe.o
	g++ -O2 -I$(FATHOM) main.cpp tbprobe.o -o main

clean:
	rm -f main *.o
