CC= gcc
CFLAGS = -DDEBUG -DWIN32 -Wall -std=c99 -pedantic -Wextra
LDFLAGS = -lmingw32 -lSDLmain -lSDL -lSDL -lopengl32 -lglu32

C_FILES := main.c event.c camera.c util.c world.c hud.c
OBJS := main.o event.o camera.o util.o world.o hud.o startup.o

all: cubeengine heightmap2wrl 

tests: utiltest

cubeengine: $(OBJS) 
	$(CC) -o $@ $^ glee.lib $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< 

clean: FRC
	rm -f cubeengine.exe $(OBJS) 

startup.o: startup.c.template gen_startup.sh $(C_FILES)
	bash gen_startup.sh
	$(CC) $(CFLAGS) -c -o startup.o startup.c

utiltest: utiltest.c util.c
	gcc -std=c99 -o utiltest $(LDFLAGS) utiltest.c util.c

heightmap2wrl: heightmap2wrl.c
	gcc -std=c99 -o heightmap2wrl heightmap2wrl.c -lmingw32 -lSDLmain -lSDL

FRC:
