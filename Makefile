CC=g++
CFLAGS=-c -Wall -g
LDFLAGS=
SOURCES=sampler.cpp objloader.cpp world.cpp shader.cpp main.cpp texture.cpp opengl_util.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=render

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) -g -framework OpenGL -framework GLUT $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
	rm render
