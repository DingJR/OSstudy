#include <sys/wait.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <termios.h>
#include <ctype.h>
#include "mem.h"
#define FOR(i,a) for(int i=0;i<a;i++)
#define DFOR(i,a) for(int i=a;i>=0;i--)
#define SUCCESS 0
#define FAIL -1
#define MAGIC 12345678
#define IMFREE 0 
#define IMNOTFREE -1
#define MAXINT 0x3f3f3f3f

typedef struct __node_t {
    int size;
    struct __node_t *next;
} node_t;

typedef struct __header_t {
    int size;
    int magic;
} header_t;


// Low level mechanism's head, all the memory allocated managed by MemHead, free memory
node_t *memHead=NULL;
int  wholesz = 0;
void *origin=NULL;

int m_error;
static void SetError(int type)
{
    m_error = type;
}

static header_t* FindFreeSpace(int Type,int sz)
{
    void *fs=memHead;
    node_t *prefs = NULL;
    int crtsz = 0;
    int lstsz = Type == M_BESTFIT? MAXINT:0;
    node_t *preCmFs = NULL;               //pre conformed-found space point
    switch(Type)
    {
        case M_BESTFIT:
        case M_WORSTFIT:
            while(fs!=NULL)
            {
                prefs = (node_t*)fs;
                crtsz = -1 * ((int)sizeof(header_t));
                while(fs!=NULL)
                {
                    node_t *tfs = (node_t*)fs;
                    crtsz += tfs->size + sizeof(node_t) ;
                    //printf("%p  %p \n",(fs+sizeof(node_t)+tfs->size),tfs->next);
                    if(tfs->next!=NULL&&(fs+sizeof(node_t)+tfs->size)!=tfs->next)
                    {
                        fs = tfs->next;
                        break;
                    }
                    fs = tfs->next;
                }
                if(crtsz >= sz && ( (Type == M_BESTFIT && crtsz<lstsz )||(Type == M_WORSTFIT && crtsz > lstsz)))
                {
                    preCmFs = prefs;
                    lstsz   = crtsz;
                }
            }
            
            prefs = preCmFs ;
            if(crtsz < sz)return NULL;
            break;
        case M_FIRSTFIT:
            prefs = (node_t*)fs;
            crtsz = -1 * ((int)sizeof(header_t));

            while(fs!=NULL)
            {
               node_t *tfs = (node_t*)fs;
               crtsz += tfs->size + sizeof(node_t) ;
               if(crtsz >= sz)break;
               if(tfs->next!=NULL&&(fs+sizeof(node_t)+tfs->size)!=tfs->next)
               {
                   prefs = tfs->next;
                   crtsz = -1 * ((int)sizeof(header_t));
               }
               fs = tfs->next;
            }
            if(crtsz < sz)return NULL;
            break;
    }
    return (header_t*)prefs;
}

static void Set_Free_Tag(void* in,int allctSize)
{
    node_t *hs      = (node_t*) in;
    node_t *headit  = (node_t*) memHead;
    header_t *headTag = (header_t*) in;

    // find the block to break, hs points to it
    // len is all the space that can be used in these blocks
    // allctSize is target space 
    int len = hs->size - (int)sizeof(header_t) + (int)sizeof(node_t);
    while(len < allctSize)
    {
        hs  =  hs->next;
        len += hs->size + (int)sizeof(node_t);
    }
    len  -= hs->size + ((int)sizeof(node_t));
    int prelen = len;

    //now len is shift from hs and it may be a breaker
    len =  ( allctSize - len );

    // h points to next free point after break, hs points to block needed break(hs == NULL if not break)
    void *th = NULL;
    //if enough to break the block ,the former to allocate, the latter to store a node_t and free space
    if(len < hs->size )
    {
        th  =  hs ;
        th  += len ;
    }
    node_t *h = (node_t*)th;

    // find prefs aftfs pointing to hs
    node_t *prefs = NULL;
    while(headit != (node_t*)headTag && headit->next != (node_t*)headTag)
        headit = headit->next;
    if(headit != memHead)
        prefs = headit;
    node_t *aftfs = hs->next; 
    //printf("%p  %p  %p  %p\n",prefs,hs,h,aftfs);

    // oiganize last, next point
    if( h != NULL ) 
    {
        h->next = aftfs;
        h->size = hs->size - len;
        //printf("h-size is : %d\n",h->size);
        headTag->size = prelen + len;
    }
    else
    {
        headTag->size = prelen + hs->size + ((int)sizeof(node_t));
    }
    //prefs != NULL
    if( prefs != NULL && h != NULL ) prefs->next = h;
    else if( prefs != NULL)          prefs->next = aftfs;
    //prefs == NULL
    else if( h != NULL)              memHead     = h;
    else                             memHead     = aftfs;
}

int mem_init(int  size_of_region)
{
    if(size_of_region <= 0 || memHead!=NULL)
    {
       SetError(E_BAD_ARGS);
       return FAIL;
    }
    int pgSz = getpagesize();
    memHead = (node_t*)mmap(NULL, pgSz , PROT_READ|
            PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
    wholesz = size_of_region;
    memset(memHead,0,pgSz);
    memHead->size = pgSz - sizeof(node_t);
    memHead->next = NULL;
    size_of_region -= memHead->size;
    node_t *prenext = memHead;
    while(size_of_region>0)
    {
        node_t *next= (node_t*)mmap(NULL, pgSz , PROT_READ|
                PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);
        memset(next,0,pgSz);
        next->size = pgSz - sizeof(node_t);
        next->next = prenext;
        size_of_region -= next->size;
        prenext = next;
    }
    memHead = prenext;
    origin = memHead;
    return SUCCESS;
}

void * mem_alloc(int sz,int style)
{
    if(sz<=0) 
    {
        SetError(E_BAD_ARGS);    
        return NULL;
    }
    /* for management's convenience, 8|size)*/
    if(sz%8) sz = 8+8*(sz/8);
    header_t *ptr = (header_t*)FindFreeSpace(style,sz);
    if(ptr==NULL)
    {
        SetError(E_NO_SPACE);
        return ptr;
    }
    Set_Free_Tag(ptr,sz);
    ptr->magic = MAGIC;
    ptr = ptr + 1;
    return ptr;
}

int mem_free(void * in)
{
    // hptr point to the header_t containing magic and size
    // ptr point to the node_t after free
    node_t* ptr = (node_t*)in;
    if(ptr==NULL)
    {
        SetError(E_BAD_POINTER);
        return FAIL;
    }
    header_t *hptr = (header_t*)(in);
    hptr = hptr - 1;
    ptr = (node_t*)hptr;
    if( hptr->magic != MAGIC )
    {
        SetError(E_BAD_POINTER);
        return FAIL;
    }

    if(memHead == NULL)
    {
        ptr->size = hptr->size -sizeof(node_t) + sizeof(header_t); 
        ptr->next = NULL;
        memHead = ptr;
        return SUCCESS;
    }

    node_t *temp = memHead;
    ptr->size = hptr->size -sizeof(node_t) + sizeof(header_t); 
    if(ptr<temp)
    {
       ptr->next = memHead;
       memHead = ptr;
       return SUCCESS;
    }
    while( temp->next != NULL ) 
    {
        if( ptr>temp && ptr<temp->next)break;
        temp=temp->next;
    }
    if(temp->next == NULL)
    {
        ptr->next = NULL;
        temp->next = ptr;
    }
    else
    {
        ptr->next = temp->next;
        temp->next = ptr;
    }
    return SUCCESS;
}

void mem_dump()
{
    node_t *temp = memHead;
    puts("Free Memory!");
    printf("head            next           Size\n");
    while(temp!=NULL)
    {
        printf("%15p%15p%5d\n",temp,temp->next,temp->size);
        temp = temp->next;
    }
    puts("Used Memory!");
    int *start = origin;
    int i;
    for(i=0;i<wholesz/4;i++)
    {
        if((int)(*(start+i+1)) == MAGIC)
        {
            printf("%p  %d\n",start+i,((int)(*(start+i))));
            printf("%p  %d\n",start+i+1,((int)(*(start+i+1))));
        }
    }
    puts("");
}

