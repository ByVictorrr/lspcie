CFLAGS=-g -O0

filter_array: json.o
filter_array.o: filter_array.c json.h
json.o: json.c json.h


%: %.o
	$(CC) $^ -lm -o $@

clean:
	rm -f *.o main
