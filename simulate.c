#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define NUMMEMORY 65536 /* maximum number of words in memory */
#define NUMREGS 8 /* number of machine registers */
#define MAXLINELENGTH 1000
#define MAXNUMBERBLOCK 256
#define MAXSIZEBLOCK 256
enum actionType
{cacheToProcessor, processorToCache, memoryToCache, cacheToMemory,
    cacheToNowhere};
typedef struct stateStruct {
    int pc;
    int mem[NUMMEMORY];
    int reg[NUMREGS];
    int numMemory;
} stateType;

typedef struct cachStruct{
    int valid[MAXNUMBERBLOCK];
    int dirty[MAXNUMBERBLOCK];
    int tag[MAXNUMBERBLOCK];
    int data[MAXNUMBERBLOCK][MAXSIZEBLOCK];
    int lru[MAXNUMBERBLOCK];
    int blockSizeInWords;
    int numberOfSets;
    int blocksPerSet;
    int numberofblocks;
}cachType;

int convertNum(int num);
void printState(stateType *);
int storegetFirstaddress(cachType *cach, int index);
int loadgetFirstadress(int address,int blockSizeInWords);
int getBlockOffset(int address, int blockSizeInWords, int numberOfSets, int blocksPerSet);
int getSetIndex(int address, int blockSizeInWords, int numberOfSets, int blocksPerSet);
void printAction(int address, int size, enum actionType type);
void store(stateType *state,cachType *cach, int address,int data);
int load(stateType *state,cachType *cach, int address);
int blockoffset(int blockSizeInwords);
int getsetbits(int numberOfSets);
int getTag(int address, int blockSizeInWords, int numberOfSets, int blocksPerSet);


int main(int argc, char *argv[])
{
    int i;
    char line[MAXLINELENGTH];
    stateType state;
    FILE *filePtr;

    if (argc != 5) {
        printf("error: usage: %s <machine-code file>\n", argv[0]);
        exit(1);
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL) {
        printf("error: can't open file %s", argv[1]);
        perror("fopen");
        exit(1);
    }
    for(i=0;i<NUMREGS;i++){
        state.reg[i]=0;
    }
    /* read in the entire machine-code file into memory */
    for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr) != NULL;
         state.numMemory++) {

        if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
            printf("error in reading address %d\n", state.numMemory);
            exit(1);
        }
        //printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]);
    }
    int temp,temp2;
    int reg1,reg2,reg3;
    int offset;
    int instruction=0;
    state.pc=0;
    cachType cach;
    cach.blockSizeInWords=atoi(argv[2]);
    cach.numberOfSets=atoi(argv[3]);
    cach.blocksPerSet=atoi(argv[4]);
    cach.numberofblocks=cach.numberOfSets*cach.blocksPerSet;
    for(int i=0;i<MAXNUMBERBLOCK;i++){
        cach.dirty[i]=0;
        cach.valid[i]=0;
        cach.lru[i]=cach.blocksPerSet-1;
    }
    while(1){
        reg1=0;
        reg2=0;
        reg3=0;
        temp=0;
        temp2=0;
        offset=0;
        int result;
        result=load(&state,&cach,state.pc);
        temp=result;
        temp2=result;
        temp2=temp2>>22;
        if(temp2==0){
            printState(&state);
            state.pc++;
            instruction++;
            reg1=(temp>>19)&7;
            reg2=(temp>>16)&7;
            reg3=temp&7;
            state.reg[reg3]=state.reg[reg1]+state.reg[reg2];
        }
        else if(temp2==1){
            printState(&state);
            state.pc++;
            instruction++;
            reg1=(temp>>19)&7;
            reg2=(temp>>16)&7;
            reg3=temp&7;
            state.reg[reg3]=~(state.reg[reg1]|state.reg[reg2]);

        }
        else if(temp2==2){
            printState(&state);
            state.pc++;
            instruction++;
            reg1=(temp>>19)&7;
            reg2=(temp>>16)&7;
            temp=temp&(0x00FFFF);
            offset=convertNum(temp);
            result=load(&state,&cach,state.reg[reg1]+offset);
            state.reg[reg2]=result;
        }
        else if(temp2==3){
            printState(&state);
            state.pc++;
            instruction++;
            reg1=(temp>>19)&7;
            reg2=(temp>>16)&7;
            temp=temp&(0x00FFFF);
            offset=convertNum(temp);
            store(&state,&cach,state.reg[reg1]+offset,state.reg[reg2]);
        }
        else if(temp2==4){
            printState(&state);
            state.pc++;
            instruction++;
            reg1=(temp>>19)&7;
            reg2=(temp>>16)&7;
            temp=temp&(0x00FFFF);
            offset=convertNum(temp);
            if(state.reg[reg1]==state.reg[reg2]){
                state.pc=state.pc+offset;

                continue;
            }

        }
        else if(temp2==5){
            printState(&state);
            state.pc++;
            instruction++;
            reg1=(temp>>19)&7;
            reg2=(temp>>16)&7;
            state.reg[reg2]=state.pc;
            state.pc=state.reg[reg1];
        }
        else if(temp2==6){
            printState(&state);
            state.pc++;
            instruction++;
            break;
        }
        else if(temp2==7){
            printState(&state);
            state.pc++;
            instruction++;
        }

    }
    /*printf("machine halted\n");
    printf("total of %d instructions executed\n",instruction);
    printf("final state of machine:\n");*/
    printState(&state);
    return(0);
}

void
printState(stateType *statePtr)
{/*
    int i;
    printf("\n@@@\nstate:\n");
    printf("\tpc %d\n", statePtr->pc);
    printf("\tmemory:\n");
    for (i=0; i<statePtr->numMemory; i++) {
        printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]);
    }
    printf("\tregisters:\n");
    for (i=0; i<NUMREGS; i++) {
        printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]);
    }
    printf("end state\n");
    */
}

int convertNum(int num){
    if(num&(1<<15)){
        num-=(1<<16);
    }
    return(num);
}
int storegetFirstaddress(cachType *cach, int index){
    int first;
    int set=index/cach->blocksPerSet;
    first=(cach->tag[index]<<getsetbits(cach->numberOfSets))+set;
    first=first<<(blockoffset(cach->blockSizeInWords));
    return first;
}

int loadgetFirstadress(int address,int blockSizeInWords){
    int i=0;
    while((address-i)%blockSizeInWords!=0){
        i++;
    }
    return (address-i);
}

int getBlockOffset(int address, int blockSizeInWords, int numberOfSets, int blocksPerSet){
    return address&((1<<blockoffset(blockSizeInWords))-1);
}

int getSetIndex(int address, int blockSizeInWords, int numberOfSets, int blocksPerSet){
    address=address>>blockoffset(blockSizeInWords);
    return address&((1<<getsetbits(numberOfSets))-1);
}
int getTag(int address, int blockSizeInWords, int numberOfSets, int blocksPerSet){
    address=address>>(blockoffset(blockSizeInWords)+getsetbits(numberOfSets));
    return address;
}
int getsetbits(int numberOfSets){
    return log2(numberOfSets);
}
int blockoffset(int blockSizeInwords){
    return log2(blockSizeInwords);
}

int load(stateType *state,cachType *cach, int address){

    int tag=getTag(address,cach->blockSizeInWords,cach->numberOfSets,cach->blocksPerSet);
    int set=getSetIndex(address,cach->blockSizeInWords,cach->numberOfSets,cach->blocksPerSet);
    int offset=getBlockOffset(address,cach->blockSizeInWords,cach->numberOfSets,cach->blocksPerSet);
    int index;
    int hit = -1;
    for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
        if (tag == cach->tag[i] && cach->valid[i] == 1) {
            hit=1;
            index=i;
        }
    }
    if(hit==-1){
        int valid=-1;
        int index2;
        int insert;
        for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
            if (cach->valid[i]==0) {
                valid=1;
                index2=i;
                break;
            }
        }
        if(valid==1){
            insert=index2;
            for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
                if (cach->lru[i]<cach->lru[index2]) {
                    cach->lru[i]++;
                }
            }
            cach->lru[index2]=0;
            cach->valid[index2]=1;
        }
        else{
            for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
                if(cach->lru[i]==cach->blocksPerSet-1){
                    insert=i;
                }
            }
            for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
                if (cach->lru[i]<cach->lru[insert]) {
                    cach->lru[i]++;
                }
            }
            if(cach->dirty[insert]==1){
                int first=storegetFirstaddress(cach,insert);
                for(int i=0;i<cach->blockSizeInWords;i++){
                    state->mem[first+i]=cach->data[insert][i];
                }
                cach->dirty[insert]=0;
                printAction(first,cach->blockSizeInWords,cacheToMemory);
            }
            else{
                int first=storegetFirstaddress(cach,insert);
                printAction(first,cach->blockSizeInWords,cacheToNowhere);
            }
            cach->lru[insert]=0;
        }
        int first=loadgetFirstadress(address,cach->blockSizeInWords);
        for(int i=0;i<cach->blockSizeInWords;i++){
            cach->data[insert][i]=state->mem[first+i];
        }
        printAction(first,cach->blockSizeInWords,memoryToCache);
        cach->tag[insert]=tag;
        printAction(address,1,cacheToProcessor);
        return cach->data[insert][offset];
    }
    else{
        for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
            if (cach->lru[i]<cach->lru[index]) {
                cach->lru[i]++;
            }
        }
        cach->lru[index]=0;
        printAction(address,1,cacheToProcessor);
        return cach->data[index][offset];
    }
}
void store(stateType *state,cachType *cach, int address,int data) {
    int tag=getTag(address,cach->blockSizeInWords,cach->numberOfSets,cach->blocksPerSet);
    int set=getSetIndex(address,cach->blockSizeInWords,cach->numberOfSets,cach->blocksPerSet);
    int offset=getBlockOffset(address,cach->blockSizeInWords,cach->numberOfSets,cach->blocksPerSet);
    int index;
    int hit = -1;
    for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
        if (tag == cach->tag[i] && cach->valid[i] == 1) {
            hit=1;
            index=i;
        }
    }
    if(hit==-1){
        int valid=-1;
        int index2;
        int insert;
        for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
            if (cach->valid[i]==0) {
                valid=1;
                index2=i;
                break;
            }
        }
        if(valid==1){
            insert=index2;
            for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
                if (cach->lru[i]<cach->lru[index2]) {
                    cach->lru[i]++;
                }
            }
            cach->lru[index2]=0;
            cach->valid[index2]=1;
        }
        else{
            for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
                if(cach->lru[i]==cach->blocksPerSet-1){
                    insert=i;
                }
            }
            for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
                if (cach->lru[i]<cach->lru[insert]) {
                    cach->lru[i]++;
                }
            }
            if(cach->dirty[insert]==1){
                int first=storegetFirstaddress(cach,insert);
                for(int i=0;i<cach->blockSizeInWords;i++){
                    state->mem[first+i]=cach->data[insert][i];
                }
                cach->dirty[insert];
                printAction(first,cach->blockSizeInWords,cacheToMemory);
            }
            else{
                int first=storegetFirstaddress(cach,insert);
                printAction(first,cach->blockSizeInWords,cacheToNowhere);
            }
            cach->lru[insert]=0;
        }
        int first=loadgetFirstadress(address,cach->blockSizeInWords);
        for(int i=0;i<cach->blockSizeInWords;i++){
            cach->data[insert][i]=state->mem[first+i];
        }
        printAction(first,cach->blockSizeInWords,memoryToCache);
        cach->tag[insert]=tag;
        cach->dirty[insert]=1;
        cach->data[insert][offset]=data;
        printAction(address,1,processorToCache);
    }
    else{
        for (int i=set*cach->blocksPerSet; i<(set+1)*cach->blocksPerSet; i++) {
            if (cach->lru[i]<cach->lru[index]) {
                cach->lru[i]++;
            }
        }
        cach->lru[index]=0;
        cach->data[index][offset]=data;
        cach->dirty[index]=1;
        printAction(address,1,processorToCache);
    }
}

/*
 * Log the specifics of each cache action.
 *
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 * 	cacheToProcessor: reading data from the cache to the processor
 * 	processorToCache: writing data from the processor to the cache
 * 	memoryToCache: reading data from the memory to the cache
 * 	cacheToMemory: evicting cache data by writing it to the memory
 * 	cacheToNowhere: evicting cache data by throwing it away
 */
void
printAction(int address, int size, enum actionType type)
{
    printf("@@@ transferring word [%d-%d] ", address, address + size - 1);
    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    } else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    } else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    } else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    } else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
}