ccc: ccc.c vector.c

test: ccc
	./test.sh

clean:
	rm -f ccc *.o *~ temp*
