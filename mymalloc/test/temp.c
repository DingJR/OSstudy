#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <malloc.h>
#include <pwd.h>
#include <termios.h>
#include <ctype.h>

typedef struct __myt_t{
    char * aa;
}myt_t;

typedef struct __node_t {
        int a[10];
} node_t;

typedef struct __header_t {
        int size;
        int magic;
} header_t;

int main()
{
    int a[10];
    printf("%d   %d  %d\n", sizeof(myt_t),sizeof(node_t),sizeof(a));
    return 0;
}
