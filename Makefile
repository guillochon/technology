CC = clang++
CCFLAGS =

SOURCES=$(wildcard src/*.cpp)
OBJECTS=$(patsubst src/%.cpp, bin/%.o, $(SOURCES))

EXEC_NAME = technology
EXEC_PATH = bin/technology

MACAPP = Technology.app

SDL2 = bin/$(MACAPP)/Contents/Resources/SDL2.framework

$(MACAPP): $(EXEC_PATH) $(SDL2)
	mkdir -p bin/$(MACAPP)/Contents/{MacOS,Resources}
	cp $(EXEC_PATH) bin/$(MACAPP)/Contents/MacOS/$(EXEC_NAME)
	cp Info.plist bin/$(MACAPP)/Contents/

$(SDL2): $(EXEC_PATH)
	mkdir -p bin/$(MACAPP)/Contents/{MacOS,Resources}
	cp -R "/Library/Frameworks/SDL2.framework" bin/$(MACAPP)/Contents/Resources/

$(EXEC_PATH): $(OBJECTS)
	mkdir -p bin
	$(CC) $(CCFLAGS) -o $@ $(OBJECTS) -std=c++17 -L/usr/local/lib -l SDL2-2.0.0

$(OBJECTS): bin/%.o : src/%.cpp
	$(CC) $(CCFLAGS) -c $< -std=c++17 -o $@

clean:
	rm -f bin/*.o
	rm -f $(EXEC_PATH)
	rm -rf bin/$(MACAPP)
