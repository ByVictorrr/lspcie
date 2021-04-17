CFLAGS=-g -O0

main: json.o
main.o: main.c json.h
json.o: json.c json.h


%: %.o
	$(CC) $^ -lm -o $@

clean:
	rm -f *.o main