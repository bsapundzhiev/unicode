CC=gcc
CFLAGS=-c -g -Wall -D_UNICODE_TEST
LDFLAGS=
SOURCES=unicode.c test.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=unicode

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@ 

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
format:
	astyle --style=stroustrup -s4 $(SOURCES)
