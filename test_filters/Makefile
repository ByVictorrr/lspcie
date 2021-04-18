CFLAGS=-g -O0

filter_array: json-parser.o
filter_array.o: filter_array.c json-parser.h
json-parser.o: json-parser.c json-parser.h


%: %.o
	$(CC) $^ -lm -o $@

clean:
	rm -f *.o main
