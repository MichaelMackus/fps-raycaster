PROGRAM = raycast
OBJS = main.o draw.o game.o
CCFLAGS = -I/usr/include/SDL2
LDFLAGS = -lSDL2

$(PROGRAM): $(OBJS)
	gcc -o $(PROGRAM) $(CCFLAGS) $(OBJS) $(LDFLAGS)

%.o: %.c
	gcc $(CCFLAGS) -c -o $@ $<

clean:
	rm *.o
	rm $(PROGRAM)
