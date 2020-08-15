#include "cache.h"
#define CACHE_NUM 10

static int readcnt;
static sem_t mutex, w;
static struct cache cache_array[CACHE_NUM];


void init_cache(struct cache *cache_block, int buffer_size)
{
    void *space = Malloc(sizeof(char) * buffer_size);
    cache_block->buffer = (char *)(space);
    cache_block->valid = 0;
    cache_block->frequency = 0;
    cache_block->content_length = 0;
    cache_block->request = NULL;
}

void init_caches(struct cache* cache_array)
{
    for (size_t i = 0; i < CACHE_NUM; i++)
    {
        init_cache(&cache_array[i], MAX_OBJECT_SIZE);
    }
}

int request_length(char *request)
{
    char *end = strstr(request, "\r\n\r\n");
    int length = end - request + 4;
    return length;
}



void update_frequency(struct cache* free_cache)
{
    int max_frequency = -1;
    for (size_t i = 0; i < CACHE_NUM; i++)
    {
        /* code */
        struct cache* cache_ptr = &cache_array[i];
        if (cache_ptr->valid == 1) {
            if (cache_ptr->frequency > max_frequency) {
                max_frequency = cache_ptr->frequency;
            }
        }
    }
    if (max_frequency == -1) {
        free_cache->frequency = 1;
    } else {
        free_cache->frequency = max_frequency + 1;
    }
    
}


int read_cache(char *request, char *dest)
{
    

    int request_L = request_length(request);
    for (size_t i = 0; i < CACHE_NUM; i++)
    {
        struct cache* cache_ptr = &cache_array[i];
        if (cache_ptr->valid == 1) {
            int cmp = memcmp(cache_ptr->request, request, request_L);
            if (cmp == 0) {
                memcpy((void *)dest, (void *)cache_ptr->buffer, cache_ptr->content_length);
                return cache_ptr->content_length;
            }
        }
    }

    return -1;
}

void free_cache(struct cache *cache_block);
struct cache* LRU_cache()
{
    int min_frequency = 0x7fffffff;
    struct cache* min_cache = NULL;
    for (size_t i = 0; i < CACHE_NUM; i++)
    {
        /* code */
        struct cache* cache_ptr = &cache_array[i];
        if (cache_ptr->valid == 1 && cache_ptr->frequency < min_frequency) {
            min_frequency = cache_ptr->frequency;
            min_cache = cache_ptr;
        }
    }
    //free_cache(min_cache);
    return min_cache;
    
}

void write_cache(char *request, char *content, int content_length)
{
    // write cache

    struct cache* empty_cache = NULL;
    for (size_t i = 0; i < CACHE_NUM; i++)
    {
        /* code */
        struct cache* cache_ptr = &cache_array[i];
        if (cache_ptr->valid == 0) {
            empty_cache = cache_ptr;
            break;
        }
    }

    
    /*
    zui kui huo shou!
    else {
        free_cache(empty_cache);
    }
    */
    if (empty_cache == NULL) {
        struct cache* min_cache = LRU_cache();
        empty_cache = min_cache;

    }
    free_cache(empty_cache);
    init_cache(empty_cache, content_length);
    int request_L = request_length(request);
    empty_cache->request = (char *)(Malloc(sizeof(char) * request_L));
    strncpy(empty_cache->buffer, content, content_length);
    strncpy(empty_cache->request, request, request_L);
    // big-endian ???
    update_frequency(empty_cache);
    empty_cache->valid = 1;
    empty_cache->content_length = content_length;

    
    

    

}



void free_cache(struct cache *cache_block)
{
    Free(cache_block->buffer);
    Free(cache_block->request);
    cache_block->valid = 0;
    cache_block->content_length = 0;
    cache_block->frequency = 0;
}


void init(void)
{
    Sem_init(&mutex, 0, 1);
    Sem_init(&w, 0, 1);
    init_caches(cache_array);
    readcnt = 0;
}

int reader(char *request, char *dest)
{
    
    P(&mutex);
    readcnt++;
    if (readcnt == 1) {
        P(&w);
    }
    V(&mutex);

    // critical session

    int length = read_cache(request, dest);
    
    P(&mutex);
    readcnt--;
    if (readcnt == 0) {
        V(&w);
    }
    V(&mutex);
    return length;
}


void writer(char *request, char *content, int content_length)
{
    P(&w);
    write_cache(request, content, content_length);
    V(&w);
}

void deinit_caches()
{
    for (size_t i = 0; i < CACHE_NUM; i++)
    {
        /* code */
        free_cache(&cache_array[i]);
    }
    
}

