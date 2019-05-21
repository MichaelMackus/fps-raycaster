PROGRAM = raycast
OBJS = main.o draw.o game.o
# CCDEFINES = -DDEBUG
CCFLAGS = -I/usr/include/SDL2 $(CCDEFINES)
LDFLAGS = -lSDL2 -lm

$(PROGRAM): $(OBJS)
	gcc -o $(PROGRAM) $(CCFLAGS) $(OBJS) $(LDFLAGS)

%.o: %.c
	gcc $(CCFLAGS) -c -o $@ $<

clean:
	rm *.o
	rm $(PROGRAM)
