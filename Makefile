CC=g++
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=shader.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=render

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) -framework OpenGL -framework GLUT $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o
	rm render
