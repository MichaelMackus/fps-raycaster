PROGRAM = raycast
OBJS = main.o draw.o game.o
# CCDEFINES = -DGAME_DEBUG
# CCFLAGS = -fsanitize=address -O2 -Wall -I/usr/include/SDL2 $(CCDEFINES)
CCFLAGS = -O2 -Wall -I/usr/include/SDL2 $(CCDEFINES)
LDFLAGS = -lSDL2 -lSDL2_image -lm

$(PROGRAM): $(OBJS)
	gcc -o $(PROGRAM) $(CCFLAGS) $(OBJS) $(LDFLAGS)

%.o: %.c
	gcc $(CCFLAGS) -c -o $@ $<

clean:
	rm *.o
	rm $(PROGRAM)
