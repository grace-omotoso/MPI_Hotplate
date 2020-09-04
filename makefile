p4:	hotplate.c
	mpicc hotplate.c -Ofast p4
clean:
	rm -f p4 *.o
