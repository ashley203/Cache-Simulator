#include <stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include <limits.h>

typedef struct block{
    int validTag;
    unsigned long int order;
    unsigned long int tag;
}block;
block** cache;

int hits=0;
int misses=0;
int reads=0;
int writes=0;

int powerOfTwo(int num){
  if (num<=0){
    return 0;
  }
  else if (num==1||(num&(num-1))==0){
    return 1;
  }
  else return 0;
}

int nextVal=0;
int main(int argc, char* argv[]){
  if(argc!=6){
    printf("error\n");
    return 1;
  }
  int csize=atoi(argv[1]);
  if(powerOfTwo(csize)!=1){
    printf("error\n");
    return 1;
  }

  char* assocArg=argv[2];
  int n=-1;
  char* tok = strtok(assocArg, ":");
  if(strcmp(tok, "direct")!=0&&strcmp(tok, "assoc")!=0){
    printf("error\n");
    return 1;
  }
  tok = strtok(0, " \n");
  if (tok!=NULL){
    if(powerOfTwo(atoi(tok))!=1){
      printf("error\n");
      return 1;
    }
    else{
      n=atoi(tok);
    }
  }

  char* rpolicy=argv[3];
  if (strcmp(rpolicy,"lru")!=0 && strcmp(rpolicy,"fifo")!=0){
    printf("error\n");
    return 1;
  }

  int bsize=0;
  if(powerOfTwo(atoi(argv[4]))==0){
    printf("error\n");
    return 1;
  }
  else{
    bsize=atoi(argv[4]);
  }

  FILE* fp=fopen(argv[5],"r");
  if (fp == NULL){
    printf("error\n");
    return 1;
  }

  int numOfSets;
  if(assocArg[0]=='d'){
    numOfSets=csize/bsize;
    n=1;
  }
  else if(n==-1){
    numOfSets=1;
    n=csize/bsize;
  }
  else{
    numOfSets=csize/(bsize*n);
  }
  unsigned long int tag;
  unsigned long int set;
  int numOfOffsetBits = log2(bsize);
  int numOfSetBits = log(numOfSets)/log(2);
  char cmd;
	unsigned long int address;

	cache = (struct block **)malloc(sizeof(struct block)* numOfSets);
  for(int i = 0; i < numOfSets; i++){
    cache[i] = (struct block *)malloc(sizeof(struct block)*n);
    for(int j = 0; j < n; j++){
        cache[i][j]=(block){0,0,0};
    }
  }

  int x= fscanf(fp, "%*lx: %c %lx",&cmd,&address);
	while(x>1){
    tag = address >> (numOfOffsetBits+numOfSetBits);
    set = (address-(tag<<(numOfOffsetBits+numOfSetBits)))>>numOfOffsetBits;
    if(rpolicy[0]=='l'){
			for(int i = 0; i < n; i++){
				if(cache[set][i].tag==tag&&cache[set][i].validTag == 1){
					hits++;
					if(cmd != 'R'){writes++;}
					cache[set][i].order = nextVal;
				    nextVal=nextVal+1;
					break;
				}
				else if((n==i+1)&&cache[set][i].validTag == 1){
					reads++;
					misses++;
					if(cmd != 'R'){writes++;}
					int oldestInd = 0;
					unsigned long int oldest = nextVal;
					for(int j = 0; j < n; j++){
						if(oldest >= cache[set][j].order){
							oldest = cache[set][j].order;
							oldestInd = j;
						}
					}
          cache[set][oldestInd]=(block){1,nextVal,tag};
				  nextVal++;
					break;
				}
				else if(!cache[set][i].validTag){
					reads++;
					misses++;
					if(cmd != 'R'){writes++;}
          cache[set][i]=(block){1,nextVal,tag};
				  nextVal++;
					break;
				}
			}	
  
		}
		if(rpolicy[0]=='f'){
			for(int i = 0;i < n; i++){
				if(cache[set][i].tag==tag && cache[set][i].validTag==1){
					hits++;
					if(cmd != 'R'){writes++;}
					break;
				}
				else if((n==i+1) && cache[set][i].validTag==1){
					reads++;
					misses++;
					if(cmd != 'R'){writes++;}
          int firstInd = 0;
					unsigned long int first = nextVal;
					for(int j = 0; j < n; j++){
						if(first >= cache[set][j].order){
							first = cache[set][j].order;
							firstInd = j;
						}
					}
          cache[set][firstInd]=(block){1,nextVal,tag};
				  nextVal++;
					break;
				}
				else if(!cache[set][i].validTag){
					misses++;
					reads++;
					if(cmd != 'R'){writes++;}
          cache[set][i]=(block){1,nextVal,tag};
				  nextVal++;
					break;
				}
			}
		}
		x= fscanf(fp, "%*lx: %c %lx",&cmd,&address);
	}
  fclose(fp);
	
	for(int i = 0;i < numOfSets; i++){
		free(cache[i]);
	}
	free(cache);



  printf("Memory reads: %d\nMemory writes: %d\nCache hits: %d\nCache misses: %d\n", reads,writes,hits,misses);

  return 0;
}