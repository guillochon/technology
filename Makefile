CC = clang++
CCFLAGS = -std=c++17

SOURCES=$(wildcard src/*.cpp)
OBJECTS=$(patsubst src/%.cpp, bin/%.o, $(SOURCES))

EXEC_NAME = technology
EXEC_PATH = bin/technology

MACAPP = Technology.app
RESOURCES = bin/$(MACAPP)/Contents/Resources

SDL2 = $(RESOURCES)/SDL2.framework
SDL2_TTF = $(RESOURCES)/SDL2_ttf.framework
SDL2_GFX = $(RESOURCES)/SDL2_gfx.framework

optimized: CCFLAGS += -O3
optimized: $(MACAPP)

$(MACAPP): $(EXEC_PATH) $(SDL2) $(SDL2_TTF) $(RESOURCES)
	mkdir -p bin/$(MACAPP)/Contents/MacOS
	cp $(EXEC_PATH) bin/$(MACAPP)/Contents/MacOS/$(EXEC_NAME)
	cp Info.plist bin/$(MACAPP)/Contents/
	cp -R fonts bin/$(MACAPP)/Contents/

debug: CCFLAGS += -g
debug: $(MACAPP)

$(RESOURCES):
	mkdir -p bin/$(MACAPP)/Contents/Resources

$(SDL2): $(EXEC_PATH) $(RESOURCES)
	rsync -at /Library/Frameworks/SDL2.framework $(RESOURCES)/

$(SDL2_TTF): $(EXEC_PATH) $(RESOURCES)
	rsync -at /Library/Frameworks/SDL2_ttf.framework $(RESOURCES)/

$(SDL2_GFX): $(EXEC_PATH) $(RESOURCES)
	rsync -at /Library/Frameworks/SDL2_gfx.framework $(RESOURCES)/

$(EXEC_PATH): $(OBJECTS)
	mkdir -p bin
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS) -L/usr/local/lib -l SDL2-2.0.0 -l SDL2_ttf -l SDL2_gfx

$(OBJECTS): bin/%.o : src/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

clean:
	rm -f bin/*.o
	rm -f $(EXEC_PATH)
	rm -rf bin/$(MACAPP)

.PHONY: debug clean
