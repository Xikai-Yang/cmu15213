#include <stdio.h>
#include "csapp.h"

#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

struct cache
{
    /* data */
    int valid;
    int frequency;
    int content_length;
    char *request;
    char *buffer;
};

void init(void);
int reader(char *request, char *dest);
void writer(char *request, char *content, int content_length);
void deinit_caches();



