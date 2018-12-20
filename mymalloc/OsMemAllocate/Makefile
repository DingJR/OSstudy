mem.o:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o
	rm mem.o
	gcc -lmem -L. -o myprogram mymain.c -Wall -Werror
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.

