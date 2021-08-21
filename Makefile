PROGRAM = raycast
OBJS = main.o game.o input.o
ENGINE = engine/raycast.o engine/entity.o engine/map.o engine/texture.o
# CCDEFINES = -DGAME_DEBUG
# CCFLAGS = -fsanitize=address -O2 -Wall -I/usr/include/SDL2 $(CCDEFINES)
CCFLAGS = -O2 -Wall -Ilib -I/usr/include/SDL2 $(CCDEFINES)
LDFLAGS = -Llib/pixelgfx -lpixelgfx -lSDL2 -lSDL2_image -lm -lGL

$(PROGRAM): $(OBJS) $(ENGINE)
	make -C lib/pixelgfx
	gcc -o $(PROGRAM) $(CCFLAGS) $(OBJS) $(ENGINE) $(LDFLAGS)

%.o: %.c
	gcc $(CCFLAGS) -c -o $@ $<
engine/%.o: engine/%.c
	gcc $(CCFLAGS) -c -o $@ $<

test: test.c
	gcc $(CCFLAGS) -lSDL2 -lGL -o test test.c

clean:
	rm *.o
	rm engine/*.o
	rm $(PROGRAM)
	make -C lib/pixelgfx clean
