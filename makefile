CC=gcc
compilar=-I. -I/usr/include/SOIL -lglut -lGLU -lSOIL -lGLEW -lGL -lm
headers=
objetos = main.o 

%.o: ../%.c $(headers)
	$(CC) -o $@ -c $< $(compilar)

all: $(objetos)
	gcc -o main $^ $(compilar)

run: all
	./main

clean:
	rm *.o main
