#ifndef CSIM_H
#define CSIM_H

struct cacheLine;
struct Cache;
int cacheFIFO(struct Cache *cache, int tag, int setIndex, char instruction);
int cacheLRU(struct Cache *cache, int tag, int setIndex, char instruction);

#endif //CSIM_H
