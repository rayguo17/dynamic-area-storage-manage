#include <stdio.h>
#include <stdlib.h>
struct map{
    int a;
    struct map *next,*prev;

};
typedef struct map map_t;

void test(map_t **a, map_t **b);


int main(){
    // char *c = "9i891";
    // int m = atoi(c);
    // printf("%d",m);
    // printf("%lu\n",sizeof(int));
    // printf("%lu\n",sizeof(struct map));
    // printf("%lu",sizeof(map_t));
    map_t *a = malloc(sizeof(map_t));

    

    //a[0] = malloc(sizeof(map_t));
    printf("sucess!\n");
    map_t *b = malloc(sizeof(map_t));
    a->a = 1;
    b->a =2;
    printf("a: %d, b: %d \n",a->a,b->a);
    test(&a,&b);
    printf("a: %d, b: %d \n",a->a,b->a);

    return 0;
}

void test(map_t **a, map_t **b)
{
    map_t * tmp = *a;
    *a = *b;
    *b=tmp;
}