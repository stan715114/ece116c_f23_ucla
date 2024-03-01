#include <iostream>
#include <bitset>
#include <stdio.h>
#include<stdlib.h>
#include <string>
#include <iomanip>
using namespace std;

#define L1_CACHE_SETS 16
#define L2_CACHE_SETS 16
#define VIC_CACHE_SIZE 4
#define L2_CACHE_WAYS 8
#define MEM_SIZE 4096
#define BLOCK_SIZE 4 // bytes per block
#define DM 0 // L1 cache
#define SA 1 // L2 cache
#define FA 2 // victim cache
#define blockBit 2 // offset
#define setBit 4 // index

struct cacheBlock
{
	int tag; // you need to compute offset and index to find the tag.
	int lru_position; // for SA only
	int data; // the actual data stored in the cache/memory
	bool valid;
	// add more things here if needed
};

struct Stat
{
	int missL1; 
	int missL2; 
	int accL1;
	int accL2;
	int accVic;
	int missVic;
	// add more stat if needed. Don't forget to initialize!
};

struct IDX_OFFSET
{
	int tag;
	int idx;
	int offSet;
	int vic_tag;

};

struct CacheStatus
{
	bool existInCache;
	int cacheLevel;
	int cachePos;

};

struct CacheEvict
{
	cacheBlock block;
	int pos_set;
	int pos_way;
	bool is_evict;

};

class cache {
private:
	cacheBlock L1[L1_CACHE_SETS]; // 1 set per row.
	cacheBlock L2[L2_CACHE_SETS][L2_CACHE_WAYS]; // 8 ways per row 
	// Add your Victim cache here ...
	cacheBlock VICTIM[VIC_CACHE_SIZE]; // 4 ways fully associate
	
	Stat myStat;
	IDX_OFFSET seat;
	CacheStatus cacheStatus;
	CacheEvict L1_Evict, VIC_Evict, L2_Evict;
	// add more things here
public:
	cache();
	void controller(bool MemR, bool MemW, int data, int adr, int* myMem);
	void adrIdx(int adr);
	void cacheSearch();
	void listEvict();
	void VICTIMlru(int pos);
	void L2lru(int pos_set, int pos_way);
	void L2Clearlru(int pos_set, int pos_way);
	void promo_VIC_to_L1(int vic_pos, int l1_pos);
	void promo_L2_to_L1(int l2_pos_set, int l2_pos_way, int l1_pos);
	void storeData(int data, int adr, int* myMem);
	void loadData(int adr, int* myMem);
	void retStat();
	// add more functions here ...	
};


