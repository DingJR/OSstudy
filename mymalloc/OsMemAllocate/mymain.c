#include <stdio.h>
#include <string.h>
#include "mem.h"
#define TIMES 10

int main(int argc,char **argv)
{
    mem_init(4088*10);
    mem_dump();
    char *m[10];
    printf("init : %d\n",m_error);
    int i=0;
    int par= 4088;
    int j=0;
    for(i=0;i<TIMES;i++)
    {
        printf("Data is : %d\n",par);
        m[i] =(char*)mem_alloc(par,M_FIRSTFIT);
        if(m[i]!=NULL)
        {
        //for(j=0;j<par;j++)*(m[i]+j) = 'a';
        *(m[i]+j-1) = '\0';
        printf("%d\n",(int)strlen(m[i]));
        puts(m[i]);
        }
        
        printf("alloc %d\n",m_error);
        mem_dump();
        printf("free %d\n",m_error);
        par *= 2;
        mem_dump();
    }
        for(j=0;j<TIMES;j++)
        mem_free(m[j]);
    //mem_dump();
    return 0;
}
