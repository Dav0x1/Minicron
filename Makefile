OBJ = minicron.o tasks.o executor.o signal.c

all: minicron
minicron: $(OBJ)
	gcc $(OBJ) -o minicron

minicron.o: minicron.c tasks.h executor.h signal.h
	gcc -c minicron.c

tasks.o: tasks.c tasks.h
	gcc -c tasks.c

executor.o: executor.c executor.h tasks.h
	gcc -c executor.c

signal.o: signal.c signal.h tasks.h
	gcc -c signal.c



.PHONY: clean
clean:
	rm -f *.o minicron