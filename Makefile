ccc: main.c vector.c parse.c codegen.c

test: ccc
	./test.sh

clean:
	rm -f ccc *.o *~ temp*
