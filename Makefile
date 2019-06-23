PROGRAM = raycast
OBJS = main.o game.o input.o
ENGINE = engine/draw.o engine/raycast.o engine/entity.o engine/map.o
# CCDEFINES = -DGAME_DEBUG
# CCFLAGS = -fsanitize=address -O2 -Wall -I/usr/include/SDL2 $(CCDEFINES)
CCFLAGS = -O2 -Wall -I/usr/include/SDL2 $(CCDEFINES)
LDFLAGS = -lSDL2 -lSDL2_image -lm

$(PROGRAM): $(OBJS) $(ENGINE)
	gcc -o $(PROGRAM) $(CCFLAGS) $(OBJS) $(ENGINE) $(LDFLAGS)

%.o: %.c
	gcc $(CCFLAGS) -c -o $@ $<
engine/%.o: engine/%.c
	gcc $(CCFLAGS) -c -o $@ $<

clean:
	rm *.o
	rm engine/*.o
	rm $(PROGRAM)
