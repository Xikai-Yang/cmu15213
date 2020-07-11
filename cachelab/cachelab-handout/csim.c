#include "cachelab.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
/*
  important note:
  followings are the mistakes I have taken when implementing this simulator
  1. please use an integer to represent time instead of time function in "time.h"
  2. do not forget to update the "time in cache Line" when there is a hit!
*/

long bitOffSets(long address, int b) {
    long minusOne = -1;
    minusOne = minusOne << b;
    minusOne = ~minusOne;
    long ans = (address << (64 - b)) >> (64 - b);
    ans = ans & minusOne;
    return ans;
}

long getSetIndex(long address, int b, int s) {
    long minusOne = -1;
    int shift = b + s;
    minusOne = minusOne << s;
    minusOne = ~minusOne;
    long ans = (address << (64 - shift)) >> (64 - s);
    ans = ans & minusOne;
    return ans;
}

long getTag(long address, int b, int s) {
    int shift = b + s;
    return (address >> shift);
}

void parseParams(int argc, char *argv[],
                int *set, int *bits, int *lines, char filename[]) {
    int ch = 0;
    while ((ch = getopt(argc, argv, "s:E:b:t:")) != -1) {
        switch (ch)
        {
            case 's': *set = (*optarg - '0'); break;
            case 'E': *lines = (*optarg - '0'); break;
            case 'b': *bits = (*optarg - '0'); break;
            case 't': strcpy(filename, optarg); break;
        default:
            break;
        }
    }
    return;
}


/*
  cache line should contain several elements
  the first part is one valid bit
  the second part is tag bits
  the third part is blocks which can contain many bytes which is
  determined by the bits
*/
struct cacheLine
{
    int validBit;
    long tag;
    int timeStamp;
    /* data */
};



void initCache(int row, int col, struct cacheLine cachePointer[row][col]) {
    for (size_t i = 0; i < row; i++)
    {
        for (size_t j = 0; j < col; j++)
        {
            cachePointer[i][j].timeStamp = 0;
            cachePointer[i][j].tag = 0;
            cachePointer[i][j].validBit = 0;
        }
        /* code */
    }
}

int dataLoad(int s, int b, int E, 
struct cacheLine cache[][E], long address, int bytes, int time) {
    long index = getSetIndex(address, b, s);
    int benchMarkTimeStamp = pow(2, 30);
    int minIndex = 0;
    
    for (size_t i = 0; i < E; i++)
    {
        if (cache[index][i].validBit == 1) {
            if (cache[index][i].timeStamp < benchMarkTimeStamp) {
                minIndex = i;
                benchMarkTimeStamp = cache[index][i].timeStamp;
            }
            long tag = getTag(address, b, s);
            if (cache[index][i].tag == tag) {
                
                cache[index][i].timeStamp = time;
                return 1; // hit
            }
        }
        if (cache[index][i].validBit == 0) {
            
            cache[index][i].validBit = 1;
            cache[index][i].tag = getTag(address, b, s);
            cache[index][i].timeStamp = time;
            
            return -1; // miss
        }
    }
    // if still not return, use replacement policy!
    cache[index][minIndex].validBit = 1;
    cache[index][minIndex].timeStamp = time;
    cache[index][minIndex].tag = getTag(address, b, s);
    return 0; // eviction

}

void display(int value, int *hit, int *miss, int *eviction) {
    switch (value)
    {
    case 1: (*hit)++; printf("hit");break;
    case 0: (*miss) = (*miss) + 1; (*eviction) = (*eviction) + 1;printf("miss eviction");break;
    case -1: (*miss)++ ; printf("miss");break;  
    default:
        break;
    }
}

void parseInstruction(int s, int b, int E, struct cacheLine cache[s][E],
    char instruction, long address, int bytes, int time,
    int *hit, int *miss, int *eviction) {
    int returnValue = 100;
    
    if (instruction == 'I') {
        return;
    }
    printf("%c %lx ", instruction, address);
    if (instruction == 'L') {
        returnValue = dataLoad(s, b, E, cache, address, bytes, time);
        display(returnValue, hit, miss, eviction);
    }
    if (instruction == 'S') {
        returnValue = dataLoad(s, b, E, cache, address, bytes, time);
        display(returnValue, hit, miss, eviction);
    }
    if (instruction == 'M') {
        returnValue = dataLoad(s, b, E, cache, address, bytes, time);
        display(returnValue, hit, miss, eviction);
        (*hit)++;
        printf(" hit");
    }
    printf("\n");
}


int main(int argc, char *argv[])
{
    int s = 0;
    int b = 0, E = 0;
    char filename[20];
    parseParams(argc, argv, &s, &b, &E, filename);
    FILE *fp = fopen(filename, "r");
    char instruction;
    long address = 0;
    int bytes = 0;
    int row = pow(2, s);
    struct cacheLine cache[row][E];
    initCache(row, E, cache);
    int hit = 0;
    int miss = 0;
    int eviction = 0;
    int timeCounter = 1;
    while (fscanf(fp, " %c %lx,%d\n", &instruction,&address, &bytes) != EOF) {
        parseInstruction(s,b,E,cache, instruction, address, bytes, timeCounter, &hit, &miss, &eviction);
        timeCounter++;
    }
    fclose(fp);
    
    printSummary(hit, miss, eviction);
    return 0;
}
