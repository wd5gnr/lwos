sample : sample.o lwos.o
	gcc -g -o sample sample.o lwos.o

sample.o : sample.c lwos.h
	gcc -g -c -o sample.o sample.c

lwos.o : lwos.c lwos.h
	gcc -g -c -o lwos.o lwos.c
