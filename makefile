CC= gcc
CFLAGS = -Wall -std=c99 -pedantic -Wextra
LDFLAGS = -lmingw32 -lSDLmain -lSDL -lSDL -lopengl32 -lglu32

C_FILES := main.c event.c camera.c util.c
OBJS := main.o event.o camera.o util.o

all: cubeengine 

tests: utiltest

cubeengine: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 

clean: FRC
	rm -f cubeengine.exe $(OBJS) 

utiltest: utiltest.c util.c
	gcc -std=c99 -o utiltest utiltest.c util.c

FRC:
