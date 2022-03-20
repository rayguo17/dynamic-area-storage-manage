#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>
#include <unistd.h>
struct map_t {
    unsigned m_size;
    char *m_addr;
    struct map_t *next, *prior;
};
typedef struct map_t map;
map *coremap=NULL;
char prompt[] = "cmd> ";
int malloc_size;
char *initial_addr;
map *cur_map;
int total=0; //链表节点个数
int verbose;
#define SIZE sizeof(map)
#define MAXLINE 1024 /*max line size*/
#define MAXARGS 3
char *fmalloc(map *maps ,int size);
int ffree(map *maps, char *addr,int size);
void logMem(map *maps);
void printEntry(map *p);
void eval(char *cmdline);
int parseline(char *cmdline,char **argv);
//link list operation function 
void clearMap(map *maps);
map* initialMap();
map* createNode();
map* deleteNode(map *p);
int insertNode(map *maps,char *addr,unsigned int m_size);
//helper function
int max(int num, ...);
int min(int num, ...);
int memToNum(char * addr);
char *numToMem(int num);
char * Malloc(size_t size);
void unix_error(char *msg);

int main(int argc, char **argv)
{
    // if(argc != 2 ){
    //     printf("usage: %s <size>",argv[0]);
    //     return 0;
    // }
    // malloc_size = atoi(argv[1]);
    dup2(1,2);
    char cmdline[MAXLINE];
    verbose=1;
    malloc_size = 1024;
    if((initial_addr = Malloc(malloc_size))==NULL){
        unix_error("malloc failed! exit program...");
        return 0;
    }
    coremap = initialMap(initial_addr,malloc_size);
    cur_map = coremap;
    if(verbose){
        printf("initial process done!\n");
        logMem(coremap);
    }
    while(1){
        printf("%s",prompt);
        fflush(stdout);
        if((fgets(cmdline,MAXLINE,stdin) == NULL) && ferror(stdin))
            unix_error("fgets error");
        if(feof(stdin)){ //ctrl-d quit
            printf("\n");
            fflush(stdout);
            break;
        }
        eval(cmdline);
        fflush(stdout);
        fflush(stdin);
    }
    free(initial_addr);
    clearMap(coremap);
    return 0;
}

void eval(char *cmdline)
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int argc=0;
    strcpy(buf,cmdline);
    argc = parseline(buf,argv);
    if(argv[0]==NULL)
        return;
    if(strcmp(argv[0],"malloc")==0){
        if(argc != 2){
            printf("malloc command requires size argument\n");
            return;
        }
        int req_size = atoi(argv[1]);
        if(verbose){
            printf("malloc cmd for size: %d\n",req_size);   
        }
        fmalloc(cur_map,req_size);
        return ;
    }
    if(strcmp(argv[0],"free")==0){
        if(argc != 3){
            printf("free command requires <addr> <size> argument\n");
            return;
        }
        int num_addr = atoi(argv[1]);
        int free_size = atoi(argv[2]);
        if(verbose){
            printf("free address: %d, size: %d\n",num_addr,free_size);
        }
        ffree(coremap,numToMem(num_addr),free_size);
        return;

    }
    printf("command not found!");
    return;
    
}
int parseline(char *cmdline,char **argv)
{
    static char array[MAXLINE];
    char *buf = array;
    char *delim;
    int argc;
    strcpy(buf,cmdline);
    buf[strlen(buf)-1] = ' ';
    while(*buf && (*buf == ' '))
    buf++;

    argc=0;
    delim = strchr(buf,' ');
    while(delim){
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;
        delim = strchr(buf,' ');

    }
    argv[argc] = NULL;
    return argc;
}

char *fmalloc(map *maps,int size)
{

    char *result;
    //start to iterate map through cur_map
    int it_time = 0;
    if(verbose){
        printf("start entry: ");
        printEntry(maps);
    }
    
    for(maps;maps->m_size;maps=maps->next){
        if(verbose){
            printf("iterate time: %d\n",it_time);
        }
        if(maps->m_size>=size){
            if(verbose){
                printf("matched entry: ");
                printEntry(maps);
            }
            result = maps->m_addr;
            maps->m_addr+=size;
            
            if(maps->m_size == size){
                //need to delete this entry
                maps = deleteNode(maps);
            }else{
                maps->m_size -= size;
            }
            return result;
        }
        it_time++;
    }
    return NULL;

}
int ffree(map *maps,char *addr, int size)
{
    char *start_addr, *end_addr;
    start_addr=addr;
    end_addr = start_addr+size;
    //first check limit
    if(start_addr<initial_addr || start_addr>initial_addr+malloc_size){
        unix_error("space to be freed not in range!");
        return 0;
    }
    if(end_addr>initial_addr+malloc_size){
        unix_error("free space too large!");
        return 0;
    }
    map *cur,*prev,*next;
    cur = maps;
    int cnt=0;
    while(cur!=NULL){
        if(cur==cur->next){
            //only have one entry!
            if(cur->m_addr > start_addr){
                if(cur->m_addr>end_addr){
                    insertNode(cur,start_addr,size);
                    return 1;
                }else{
                    int new_size = max(2,size,cur->m_size+(cur->m_addr-start_addr));
                    cur->m_addr = start_addr;
                    cur->m_size = new_size;
                    return 1;
                }
            }else{
                if(cur->m_addr+cur->m_size < start_addr){
                    insertNode(cur,start_addr,size);
                    return 1;
                }else{
                    int new_size = max(2,cur->m_size,end_addr-cur->m_addr);
                    cur->m_size = new_size;
                    return 1;
                }
            }
        }
        //more than one entry
        if(cur->m_addr < cur->next->m_addr){
            //except for the last entry!
            if(start_addr < cur->next->m_addr && start_addr >= cur->m_addr){
                //found the gap it can fit
                map *tmp = cur->next;
                while((tmp->m_addr + tmp->m_size) < end_addr && tmp->m_addr > tmp->prior->m_addr){
                    tmp = deleteNode(tmp);
                }
                //now tmp is the end_addr gap
                //have 4 situation now, start addr in/out block,end addr in/out block
                if(start_addr < cur->m_addr + cur->m_size){
                    //inside cur block,
                    if(end_addr < tmp->m_addr){
                        int new_size = max(2,end_addr-cur->m_addr,cur->m_size);
                        cur->m_size = new_size;
                        return 1;
                    }else{
                        char *cur_addr = cur->m_addr;
                        int new_size = tmp->m_addr+tmp->m_size-cur->m_addr;
                        deleteNode(cur);
                        tmp->m_addr = cur_addr;
                        tmp->m_size = new_size;
                        return 1;
                    }
                    
                }else{
                    //start addr not inside the block
                    //
                    if(end_addr < tmp->m_addr){
                        insertNode(cur,start_addr,size);
                        return 1;
                    }else{
                        int new_size = tmp->m_size + (tmp->m_addr-start_addr);
                        tmp->m_addr = start_addr;
                        tmp->m_size = new_size;
                        return 1;
                    }
                }   
            }
        }else{
            //if are at last block
            //should determine it is too small or too big?
            if(start_addr >= cur->m_addr){
                //too big
                if(start_addr > cur->m_addr + cur->m_size){
                    insertNode(cur,start_addr,size);
                    return 1;
                }else{
                    int new_size = max(2,end_addr-cur->m_addr,cur->m_size);
                    cur->m_size = new_size;
                    return 1;
                }
            }
            if(start_addr < cur->next->m_addr){
                map *tmp = cur->next;
                while((tmp->m_addr + tmp->m_size) < end_addr ){
                    if(tmp == tmp->next){
                        //have one only
                        tmp->m_addr = start_addr;
                        tmp->m_size = size;
                        return 1;
                    }
                    tmp = deleteNode(tmp);
                }
                if(end_addr<tmp->m_addr){
                    insertNode(cur,start_addr,size);
                    return 1;
                }else{
                    int new_size = tmp->m_size + (tmp->m_addr-start_addr);
                    tmp->m_addr = start_addr;
                    tmp->m_size = new_size;
                    return 1;
                }

            }

        }
        cur = cur->next;
        cnt++;
        if(cnt>2*total){
            //avoid inifinite loop
            return 0;
        }
    }

}


int insertNode(map *maps,char *addr,unsigned int m_size)
{
    //this function is designed for linklist to insert node in specific
    //position. it will only check addr order, will not check m_size
    //we will still iterate it, but if you give us selected maps it will be faster!
    //if return 0 then didn't insert. return 1 means insert successfully.
    map *new_node = createNode();
    new_node->m_addr=addr;
    new_node->m_size = m_size;
    map *prev,*next,*cur;
    cur = maps;
    for(cur;cur!=NULL;cur = cur->next){
        if(cur==cur->next){
            //only have one entries then order doesnot matter!
            cur->next = new_node;
            cur->prior = new_node;
            new_node->prior = cur;
            new_node->next = cur;
            return 1;
        }
        //only suitable for mature linklist has more than 2 entries;
        if(cur -> m_addr > cur->next->m_addr){
            //inside the order link list
            if(cur->m_addr < addr && cur->next->m_addr > addr){
                next = cur->next;
                cur->next = new_node;
                next->prior = new_node;
                new_node->prior = cur;
                new_node->next = next;
                return 1;
            }
        }else{
            //at the border
            if(cur->m_addr < addr || cur->next->m_addr > addr){
                next = cur->next;
                cur->next = new_node;
                next->prior = new_node;
                new_node->prior = cur;
                new_node->next = next;
                return 1;
            }
        }
    }
    free(new_node);
    total--;
    return 0;

}
void logMem(map *maps)
{
    map* curr;
    curr = maps;
    int i=0;
    // for(i;i<total;i++){
    //     printf("start address: %d ",memToNum(curr->m_addr));
    //     printf("size: %d\n",curr->m_size);
    //     curr = curr->next;
    // }
    int flag=0;
    printf("area address: 0x%016" PRIXPTR "\n",(uintptr_t)initial_addr);
    printf("malloc size: %d\n",malloc_size);
    printf("--------total entries: %d-----------\n",total);
    
    while(curr != NULL){
        if(curr->next == curr){
            //only have one entry
            printf("start address: %d ",memToNum(curr->m_addr));
            printf("size: %d\n",curr->m_size);
            return ;
        }
        
        if(curr->next->m_addr < curr->m_addr && !flag){
            //head capture
            flag=1;
            curr = curr->next;
            continue;
        }
        if(flag==1){
            printf("start address: %d | ",memToNum(curr->m_addr));
            printf("size: %d | ",curr->m_size);
            printf("end address: %d\n",memToNum(curr->m_addr) + curr->m_size);
            i++;
        }
        
        if(i==total)
            break;
        curr = curr->next;
    }
    printf("----------result printing end-----------");

}
void printEntry(map *p)
{
    printf("start address : %d | size: %d\n",memToNum(p->m_addr),p->m_size);

}

map *initialMap(char * init_addr, int size)
{
    map * new_map = createNode(SIZE);
    new_map->m_addr = init_addr;
    new_map->m_size=size;
    new_map->next = new_map;
    new_map->prior = new_map;
    return new_map;
}


map* createNode()
{
    map* pn = (map *)Malloc(SIZE);
    pn->next=NULL;
    pn->prior=NULL;
    pn->m_addr=NULL;
    pn->m_size=0;
    total++;
    return pn;
}
map* deleteNode(map *p)
{
    map *prev,*next;
    prev = p->prior;
    next = p->next;
    prev->next = next;
    next->prior = prev;
    free(p);
    total--;
    return next;
}
void clearMap(map *maps)
{
    map *cur;
    if(verbose){
        printf("clear process start: %d entries need to be clean\n",total);
    }   
    
    for(cur=maps;cur!=NULL;){
        if(verbose){
            printf("%d entries | ",total);
            printf("start address: 0x%016" PRIXPTR "\n",(uintptr_t)cur->m_addr);
        }
        
        if(cur==cur->next){
            free(cur);
            maps=NULL;
            total=0;
            return ;
        }
        cur = deleteNode(cur);

    }

}

int max(int num, ...)
{
    va_list valist;
    int result=0,i=0;
    va_start(valist,num);
    for(i;i<num;i++){
        int temp = va_arg(valist,int);
        if(temp > result)
            result = temp;
    }
    va_end(valist);
    return result;
}

int min(int num, ...)
{
    va_list valist;
    int result=0,i=0;
    va_start(valist,num);
    for(i;i<num;i++){
        int temp = va_arg(valist,int);
        if(temp < result)
            result = temp;
    }
    va_end(valist);
    return result;
}

int memToNum(char * addr)
{
    return (int)(addr-initial_addr);
}
char *numToMem(int num)
{
    return initial_addr+num;
}


char *Malloc(size_t size) 
{
    char *p;

    if ((p  = malloc(size)) == NULL)
	unix_error("Malloc error");
    return p;
}

void unix_error(char *msg) /* Unix-style error */
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(0);
}