#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "c-sim.h"


struct cacheLine{
    int valid;
    int dirtyBit;
    int TAG;
    struct cacheLine *next;
};

struct Cache{
    int numSet;
    int numLines;
    int cacheSize;
    int blockSize;
    int hits;
    int misses;
    int reads;
    int writes;
    char* writePolicy;
    char* replacement;
    char* type;
    struct cacheLine **arr;
};

int cacheFIFO(struct Cache *cache, int tag, int setIndex, char instruction){
    struct cacheLine *ptr = (cache->arr[setIndex]);
    struct cacheLine *front = (cache->arr[setIndex]);
    while(ptr->next!=NULL){ //fully associative and n-way associative caches, check all lines for validity
        if(ptr->valid == -1){ //empty line
            cache->misses++;
            if(instruction == 'R'){
                cache->reads++;
            }else if(instruction == 'W'){
                cache->reads++;
                cache->writes++;
            }else{
                return 0;
            }
            ptr->valid = 1;
            ptr->TAG = tag;
            return 1;
        }else{ //valid so check TAG or continue
            if(tag == ptr->TAG){//wb vs wt write hits
                cache->hits++;
                if(instruction == 'W' && ptr->dirtyBit == -1){ //wt
                    cache->writes++;
                }else if(instruction == 'W' && (ptr->dirtyBit == 0 || ptr->dirtyBit == 1)){ //wb value as been updated
                    ptr->dirtyBit = 1;
                }
                return 1;
            }
        }
        ptr = ptr->next;
    }
    //check last line
    if(ptr->valid == -1){ //last line is empty
        cache->misses++;
        if(instruction == 'R'){
            cache->reads++;
        }else if(instruction == 'W'){
            cache->reads++;
            cache->writes++;
        }else{
            return 0;
        }
        ptr->valid = 1;
        ptr->TAG = tag;
        return 1;
    }else{ //last line is valid, check tag for hit or miss or evict
        //printf("%c\n", instruction);
        if(tag == ptr->TAG){//wb vs wt write hits
            cache->hits++;
            if(instruction == 'W' && ptr->dirtyBit == -1){
                cache->writes++;
            }else if(instruction == 'W' && (ptr->dirtyBit == 0 || ptr->dirtyBit == 1)){
                //printf("wb\n");
                ptr->dirtyBit = 1;
            }
        }else{ //eviction
            cache->misses++;
            if(instruction == 'R'){
                if(front->dirtyBit == 1){
                    cache->writes++;
                    printf("hello\n");
                }
                cache->reads++;
                front->TAG = tag;
            }if(instruction == 'W'){
                //printf("hello\n");
                if(front->dirtyBit == -1 || front->dirtyBit == 0){ //wt or if wb is not changed
                    printf("hello1\n");
                    cache->reads++;
                    cache->writes++;
                    front->TAG = tag;                  //eviction of first input item
                }else if(front->dirtyBit == 1){
                    printf("hello\n");//wb
                    cache->writes+=2;
                    cache->reads++;
                    front->TAG = tag;
                    front->dirtyBit = 0;
                }
            }
            if(ptr == front){
                return 1;
            }
            (cache->arr[setIndex]) = front->next;
            ptr->next = front;
            ptr->next->next = NULL;
        }
    }return 1;
}
int cacheLRU(struct Cache *cache, int tag, int setIndex, char instruction){
    struct cacheLine *ptr = (cache->arr[setIndex]);
    struct cacheLine *front = (cache->arr[setIndex]);
    struct cacheLine *ptr2 = front;
    while(ptr->next!=NULL){ //fully associative and n-way associative caches, check all lines for validity
        if(ptr->valid == -1){ //empty line
            cache->misses++;
            if(instruction == 'R'){
                cache->reads++;
            }else if(instruction == 'W'){
                cache->reads++;
                cache->writes++;
            }else{
                return 0;
            }
            ptr->valid = 1;
            ptr->TAG = tag;
            if(ptr == front){
                return 1;
            }
            while(ptr2->next != ptr){
                ptr2 = ptr2->next;
            }
            ptr2->next = ptr->next;
            (cache->arr[setIndex]) = ptr;
            ptr->next = front;
            return 1;
        }else{ //valid so check TAG or continue
            if(tag == ptr->TAG){//wb vs wt write hits
                cache->hits++;
                if(instruction == 'W' && ptr->dirtyBit == -1){ //wt
                    cache->writes++;
                }else if(instruction == 'W' && (ptr->dirtyBit == 0 || ptr->dirtyBit == 1)){ //wb value as been updated
                    ptr->dirtyBit = 1;
                }
                if(ptr == front){
                    return 1;
                }
                while(ptr2->next != ptr){
                    ptr2 = ptr2->next;
                }
                ptr2->next = ptr->next;
                (cache->arr[setIndex]) = ptr;
                ptr->next = front;
                return 1;
            }
        }
        ptr = ptr->next;
    }
    //check last line
    if(ptr->valid == -1){ //last line is empty
        cache->misses++;
        if(instruction == 'R'){
            cache->reads++;
        }else if(instruction == 'W'){
            cache->reads++;
            cache->writes++;
        }else{
            return 0;
        }
        ptr->valid = 1;
        ptr->TAG = tag;
        if(ptr == front){
            return 1;
        }
        while(ptr2->next != ptr){
            ptr2 = ptr2->next;
        }
        ptr2->next = ptr->next;
        (cache->arr[setIndex]) = ptr;
        ptr->next = front;
        return 1;
    }else{ //last line is valid, check tag for hit or miss or evict
        if(tag == ptr->TAG){//wb vs wt write hits
            cache->hits++;
            if(instruction == 'W' && ptr->dirtyBit == -1){
                cache->writes++;
            }else if(instruction == 'W' && (ptr->dirtyBit == 0 || ptr->dirtyBit == 1)){
                ptr->dirtyBit = 1;
            }
            if(ptr == front){
                return 1;
            }
            while(ptr2->next != ptr){
                ptr2 = ptr2->next;
            }
            ptr2->next = ptr->next;
            (cache->arr[setIndex]) = ptr;
            ptr->next = front;
            return 1;
        }else{ //eviction
            struct cacheLine *ptr3 = front;  // last item in linked list (least used)
            while(ptr3->next != NULL){
                ptr3 = ptr3->next;
            }
            cache->misses++;
            if(instruction == 'R'){
                if(ptr3->dirtyBit == 1){
                    cache->writes++;
                }
                cache->reads++;
                ptr3->TAG = tag;
            }else if(instruction == 'W'){
                if(ptr3->dirtyBit == -1 || ptr3->dirtyBit == 0){ //wt or if wb is not changed
                    cache->reads++;
                    cache->writes++;
                    ptr3->TAG = tag;
                }else if(front->dirtyBit == 1){ //wb
                    cache->writes++;
                    cache->reads++;
                    ptr3->TAG = tag;
                }
            }
            if(ptr3 == front){
                return 1;
            }
            while(ptr2->next != ptr3){
                ptr2 = ptr2->next;
            }
            ptr2->next = ptr3->next;
            (cache->arr[setIndex]) = ptr3;
            ptr3->next = front;
        }
    }return 1;
}


int main(int argc, char* argv[]){
    if(argc == 7){
        struct Cache* cache = (struct Cache*)malloc(sizeof(struct Cache));
        cache->cacheSize = atoi(argv[1]);
        char* assoc = argv[2];
        cache->type = assoc;
        int associativity = -1;
        cache->blockSize = atoi(argv[3]);
        int blockBits = log(cache->blockSize)/log(2);
        cache->replacement = argv[4];
        cache->writePolicy = argv[5];
        cache->numLines = cache->cacheSize/cache->blockSize;
        cache->hits = 0;
        cache->misses = 0;
        cache->reads = 0;
        cache->writes = 0;
        if(strcmp(assoc, "direct") == 0){
            cache->numSet = cache->numLines;
            cache->arr = (struct cacheLine**)malloc(sizeof(struct cacheLine*) * cache->numSet);
            struct cacheLine *ptr;
            int i;
            for(i = 0; i < cache->numSet; i++){
                ptr = (struct cacheLine*)malloc(sizeof(struct cacheLine));
                (cache->arr[i]) = ptr;
            }
            for(i=0; i<cache->numSet; i++){
                struct cacheLine *ptr = (cache->arr[i]);
                ptr->valid = -1;
                if(strcmp(cache->writePolicy, "wb") == 0){
                    ptr->dirtyBit = 0;
                }else if(strcmp(cache->writePolicy, "wt") == 0){
                    ptr->dirtyBit = -1;
                }else{
                    printf("error\n");
                    return 0;
                }
                ptr->TAG = '\0';
                ptr ->next = NULL;
            }

        }
        else if(strcmp(assoc, "assoc") == 0){
            cache->numSet = 1;
            cache->arr = (struct cacheLine**)malloc(sizeof(struct cacheLine*));
            struct cacheLine *ptr;
            int i = 0;
            for(i = 0; i < cache->numSet; i++){
                ptr = (struct cacheLine*)malloc(sizeof(struct cacheLine));
                (cache->arr[i]) = ptr;
            }
            ptr->valid = -1;
            ptr->TAG = '\0';
            if(strcmp(cache->writePolicy, "wb") == 0){
                ptr->dirtyBit = 0;
            }else if(strcmp(cache->writePolicy, "wt") == 0){
                ptr->dirtyBit = -1;
            }else{
                printf("error\n");
                return 0;
            }
            i = 0;
            while(i < cache->numLines){
                struct cacheLine *newLine = (struct cacheLine*)malloc(sizeof(struct cacheLine));
                newLine->valid = -1;
                newLine->dirtyBit = ptr->dirtyBit;
                newLine->TAG = '\0';
                newLine->next = NULL;
                ptr->next = newLine;
                ptr = newLine;
                i++;
            }
        }
        else{
            sscanf(assoc, "assoc:%d", &associativity);
            if(associativity == -1){
                printf("error\n");
                return 0;
            }
            cache->numSet = cache->numLines/associativity;
            cache->arr = (struct cacheLine**)malloc(cache->numSet*sizeof(struct cacheLine*));
            int i;
            struct cacheLine *ptr;
            for(i = 0; i < cache->numSet; i++){
                ptr = (struct cacheLine*)malloc(sizeof(struct cacheLine));
                (cache->arr[i]) = ptr;
            }
            int j;
            for(i = 0; i<cache->numSet; i++){
                struct cacheLine *row = cache->arr[i];
                struct cacheLine *ptr = row;
                ptr->TAG = '\0';
                ptr->valid = -1;
                ptr->next = NULL;
                if(strcmp(cache->writePolicy,"wt")==0){
                    ptr->dirtyBit = -1;
                }else if(strcmp(cache->writePolicy,"wb")==0){
                    ptr->dirtyBit = 0;
                }else{
                    printf("error\n");
                    return 0;
                }
                for(j = 1; j<associativity; j++){
                    struct cacheLine *newLine = (struct cacheLine*)malloc(sizeof(struct cacheLine));
                    newLine->valid = -1;
                    newLine->dirtyBit = ptr->dirtyBit;
                    newLine->TAG = '\0';
                    newLine->next = NULL;
                    ptr->next = newLine;
                    ptr = newLine;
                }
            }
        }
        int setBits = log(cache->numSet)/log(2);
        int mask = (1 << setBits) - 1;
        int instructionPointer;
        int address;
        int setIndex;
        int tag;
        int x;
        char instruction;
        FILE *filename = fopen(argv[6], "r");
        if(filename == NULL){
            printf("error\n");
            return 0;
        }
        while(fscanf(filename, "%x: %c %x\n", &instructionPointer, &instruction, &address) ==3){
            setIndex = (address >> blockBits) & mask;
            tag = address >> (blockBits + setBits);
            if(strcmp(cache->replacement, "FIFO") == 0){
                x = cacheFIFO(cache, tag, setIndex, instruction);
            }else if(strcmp(cache->replacement, "LRU") == 0){
                x = cacheLRU(cache, tag, setIndex, instruction);
            }else{
                printf("error\n");
                return 0;
            }
            if(x == 0){
               printf("error\n");
               return 0;
            }
        }
        printf("Memory reads: %d\n", cache->reads);
        printf("Memory writes: %d\n", cache->writes);
        printf("Cache hits: %d\n", cache->hits);
        printf("Cache misses: %d\n", cache->misses);
        free(cache);
        return 1;
    }else{
        printf("error\n");
        return 0;
    }
}
