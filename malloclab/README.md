### Malloc Lab

![Image text](https://github.com/Xikai-Yang/cmu15213/blob/master/img/malloc_performance.png)

This assignment will help you develop a detailed understanding of how malloc was implemented under the hood

In this lab, I've tried several data structures to organize the memory under the hood

it includes: implicit list / explicit list / segregated lists

And I've adopted LIFO policy to insert and delete the free block from the lists

Besides, I've also rewritten the "realloc" function to improve its performance

During the process, I've found two github repos really powerful which can achieve 92 and 98 respectively without causing any bugs

Please feel free to check them out:

https://github.com/kayoyin/malloclab/blob/master/mm_92.c

https://github.com/mightydeveloper/Malloc-Lab/blob/master/mm.c

