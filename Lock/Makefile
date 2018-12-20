spinlock.o:
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
	gcc -c -fpic spinlock.c -Wall -Werror
	gcc -shared -o libspinlock.so spinlock.o
	rm spinlock.o
	gcc -c -fpic mutex.c -Wall -Werror
	gcc -shared -o libmutex.so mutex.o
	gcc -c -fpic counter.c -Wall -Werror
	gcc -shared -o libcounter.so counter.o
	gcc -c -fpic list.c -Wall -Werror
	gcc -shared -o liblist.so list.o
	gcc -c -fpic hash.c -Wall -Werror
	gcc -shared -o libhash.so hash.o
	gcc -c -fpic twophase.c -Wall -Werror
	gcc -shared -o libtwophase.so twophase.o
	rm list.o
	rm hash.o
	rm twophase.o
	rm counter.o
	rm mutex.o
	gcc -lmutex -ltwophase -lhash -lspinlock -llist -lcounter -L. -pthread -o myprogram mymain.c -Wall -Werror

