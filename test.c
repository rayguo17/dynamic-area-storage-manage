#include <stdio.h>
#include <stdlib.h>
struct map{
    int a;
    struct map *next,*prev;

};
typedef struct map map_t;

int main(){
    // char *c = "9i891";
    // int m = atoi(c);
    // printf("%d",m);
    // printf("%lu\n",sizeof(int));
    // printf("%lu\n",sizeof(struct map));
    // printf("%lu",sizeof(map_t));
    void * m_addr = malloc(100);
    void * n_addr = m_addr+100;
    printf("%s\n",(m_addr > n_addr)?"TRUE":"FALSE");

    return 0;
}