#include "cache.h"

cache::cache()
{
	for (int i=0; i<L1_CACHE_SETS; i++)
		L1[i].valid = 0; 
	for (int i=0; i<L2_CACHE_SETS; i++)
		for (int j=0; j<L2_CACHE_WAYS; j++)
			L2[i][j].valid = 0; 
	for (int i = 0; i < VIC_CACHE_SIZE; i++)
		VICTIM[i].valid = 0;

	// Do the same for Victim Cache ...

	this->myStat.missL1 =0;
	this->myStat.missL2 =0;
	this->myStat.missVic = 0;
	this->myStat.accL1 =0;
	this->myStat.accL2 =0;
	this->myStat.accVic = 0;

	this->seat.idx = 0;
	this->seat.offSet = 0;
	this->seat.tag = 0;
	this->seat.vic_tag = 0;

	this->cacheStatus.cacheLevel = 0;
	this->cacheStatus.cachePos = 0;
	this->cacheStatus.existInCache = 0;

	this->L1_Evict.is_evict = 0;
	this->VIC_Evict.is_evict = 0;
	this->L2_Evict.is_evict = 0;

	// cout << "initialize" << endl;
	
}
void cache::controller(bool MemR, bool MemW, int data, int adr, int* myMem)
{
	// add your code here
	adrIdx(adr);
	cacheSearch();

	if (MemR) {
		listEvict();
		loadData(adr, myMem);
	}
	else if (MemW) {
		storeData(data, adr, myMem);
	}

}

void cache::adrIdx(int adr)
{
	seat.idx = (int)(((((bitset<32>)adr) & ((bitset<32>)0x3c)) >> 2).to_ulong());
	seat.offSet = (int)((((bitset<32>)adr) & ((bitset<32>)0x3)).to_ulong());
	seat.tag = (int)(((((bitset<32>)adr) & ((bitset<32>)0xffffffc0)) >> 6).to_ulong());
	seat.vic_tag = (int)(((((bitset<32>)adr) & ((bitset<32>)0xfffffffc)) >> 2).to_ulong());
	
	//cout << "adr: " << adr << endl; 
	//cout << seat.idx << endl; 
	//cout << seat.tag << endl;
	//cout << seat.vic_tag << endl;  
	
}

void cache::cacheSearch()
{
	cacheStatus.existInCache = 0;

	// search if the address in cache
	if ((L1[seat.idx].tag == seat.tag) && (L1[seat.idx].valid == 1)) { // search L1
		cacheStatus.cacheLevel = 0;
		cacheStatus.existInCache = 1;
		//cout << "adr in L1" << endl;
		return;
	}

	for (int i = 0; i < VIC_CACHE_SIZE; i++) { // search Victim
		if ((VICTIM[i].tag == seat.vic_tag) && (VICTIM[i].valid == 1)) {
			cacheStatus.cacheLevel = 1;
			cacheStatus.existInCache = 1;
			cacheStatus.cachePos = i;
			//cout << "adr in VICTIM" << endl;
			return;
		}
	}

	for (int i = 0; i < L2_CACHE_WAYS; i++) { // search L2
		if ((L2[seat.idx][i].tag == seat.tag) && (L2[seat.idx][i].valid == 1)) {
			cacheStatus.cacheLevel = 2;
			cacheStatus.existInCache = 1;
			cacheStatus.cachePos = i;
			//cout << "adr in L2" << endl;
			return;
		}
	}
}

void cache::listEvict() // store the least used data of L1, L2, and Victim
{
	// L1, copy the block if it is used already
	if (L1[seat.idx].valid == 1) {
		L1_Evict.block = L1[seat.idx];
		L1_Evict.is_evict = 1;
	}

	// VICTIM, copy the block if it is fully used
	// update evict status and position (either empty's or LRU's)
	VIC_Evict.is_evict = 1;
	for (int i = 0; i < VIC_CACHE_SIZE; i++) {

		VIC_Evict.pos_way = i;

		if ((VICTIM[i].lru_position == 0) && (VICTIM[i].valid == 1)) { // evict
			VIC_Evict.block = VICTIM[i];
			break;
		}

		if (VICTIM[i].valid == 0) { // not evict
			VIC_Evict.is_evict = 0;
			break;
		} 
	}

	// L2, locate the set index
	// either the data evicted from VICTIM or input address
	if (VIC_Evict.is_evict) {
		L2_Evict.pos_set = (int)(VIC_Evict.block.tag & 0xf);
		L2_Evict.is_evict = 1;
		for (int i = 0; i < L2_CACHE_WAYS; i++) {

			L2_Evict.pos_way = i;

			if ((L2[L2_Evict.pos_set][i].lru_position == 0) && (L2[L2_Evict.pos_set][i].valid == 1)) { // evict
				L2_Evict.block = L2[L2_Evict.pos_set][i];
				break;
			}

			if (L2[L2_Evict.pos_set][i].valid == 0) { // not evict
				L2_Evict.is_evict = 0;
				break;
			}
		}
	}
}

void cache::VICTIMlru(int pos) // bring VICTIM[pos] to the most recently used case and decrease the rest LRU
{
	int promo_lru = VICTIM[pos].lru_position;
	if (VICTIM[pos].valid != 1)
		promo_lru = 0;

	for (int i = 0; i < VIC_CACHE_SIZE; i++) {
		if (VICTIM[i].lru_position > promo_lru)
			VICTIM[i].lru_position = VICTIM[i].lru_position - 1;
	}
	VICTIM[pos].lru_position = 3;
}

void cache::L2lru(int pos_set, int pos_way) // bring L2[pos_set][pos_way] to the most recently used case and decrease the rest LRU
{
	int promo_lru = L2[pos_set][pos_way].lru_position;
	if (L2[pos_set][pos_way].valid != 1)
		promo_lru = 0;

	for (int i = 0; i < L2_CACHE_WAYS; i++) {
		if (L2[pos_set][i].lru_position > promo_lru)
			L2[pos_set][i].lru_position = L2[pos_set][i].lru_position - 1;
	}
	L2[pos_set][pos_way].lru_position = 7;
}

void cache::L2Clearlru(int pos_set, int pos_way) // clear data in L2[pos_set][pos_way] and maintain LRU consistency
{
	int promo_lru = L2[pos_set][pos_way].lru_position;
	for (int i = 0; i < L2_CACHE_WAYS; i++) { 
		if (L2[pos_set][i].lru_position < promo_lru)
			L2[pos_set][i].lru_position = L2[pos_set][i].lru_position + 1;
	}
	L2[pos_set][pos_way].valid = 0;
}

void cache::promo_VIC_to_L1(int vic_pos, int l1_pos) // swap VICTIM[] and L1[]
{
	cacheBlock dummy = L1[l1_pos];

	L1[l1_pos].valid = 1;
	L1[l1_pos].data = VICTIM[vic_pos].data;  
	L1[l1_pos].tag = (VICTIM[vic_pos].tag >> 4);

	VICTIM[vic_pos].valid = dummy.valid;
	VICTIM[vic_pos].data = dummy.data;
	VICTIM[vic_pos].tag = (dummy.tag << 4) + l1_pos;
	VICTIMlru(vic_pos);
}

void cache::promo_L2_to_L1(int l2_pos_set, int l2_pos_way, int l1_pos) // bring L2[][] to L1, take care of L1 evict and VICTIM evict if any
{
	cacheBlock dummy = L1[l1_pos];

	L1[l1_pos].valid = 1; 
	L1[l1_pos].data = L2[l2_pos_set][l2_pos_way].data;
	L1[l1_pos].tag = L2[l2_pos_set][l2_pos_way].tag;

	L2Clearlru(l2_pos_set, l2_pos_way); // clear the space in L2

	if (L1_Evict.is_evict) {
		cacheBlock dummy2 = VICTIM[VIC_Evict.pos_way];

		VICTIMlru(VIC_Evict.pos_way);
		VICTIM[VIC_Evict.pos_way].valid = 1;
		VICTIM[VIC_Evict.pos_way].tag = (dummy.tag << 4) + seat.idx;
		VICTIM[VIC_Evict.pos_way].data = dummy.data;

		if (VIC_Evict.is_evict) {
			L2lru(L2_Evict.pos_set, L2_Evict.pos_way);
			L2[L2_Evict.pos_set][L2_Evict.pos_way].valid = 1;
			L2[L2_Evict.pos_set][L2_Evict.pos_way].tag = (dummy2.tag >> 4);
			L2[L2_Evict.pos_set][L2_Evict.pos_way].data = dummy2.data;
		}
	}

	L2[l2_pos_set][l2_pos_way].valid = dummy.valid;
	L2[l2_pos_set][l2_pos_way].data = dummy.data;
	L2[l2_pos_set][l2_pos_way].tag = dummy.tag;
	L2lru(l2_pos_set, l2_pos_way);
}

void cache::storeData(int data, int adr, int *myMem)
{
	if (cacheStatus.existInCache) {
		switch (cacheStatus.cacheLevel) {
		case 0:  //  data exist in L1
			// update data in L1
			L1[seat.idx].valid = 1;
			L1[seat.idx].data = data;
			break;
		case 1: // data exist in victin
			// update data in VICTIM
			VICTIM[cacheStatus.cachePos].valid = 1;
			VICTIM[cacheStatus.cachePos].data = data;
			VICTIMlru(cacheStatus.cachePos);
			break;
		case 2: // data exist in L2
			// update data in L2
			L2[seat.idx][cacheStatus.cachePos].valid = 1;
			L2[seat.idx][cacheStatus.cachePos].data = data;
			L2lru(seat.idx, cacheStatus.cachePos);
			break;
		}
	}

	myMem[adr] = data; // unconditional write to main memory.

}

void cache::loadData(int adr, int* myMem)
{
	//cout << "load data" << endl;
	if (cacheStatus.existInCache) {
		switch (cacheStatus.cacheLevel) {
		case 0:  //  data exist in L1
			myStat.accL1++;
			break;
		case 1: // data exist in victim
			myStat.accL1++;
			myStat.missL1++;
			myStat.accVic++;
			promo_VIC_to_L1(cacheStatus.cachePos, seat.idx);
			break;
		case 2: // data exist in L2
			myStat.accL1++;
			myStat.missL1++;
			myStat.accVic++;
			myStat.missVic++;
			myStat.accL2++;
			promo_L2_to_L1(seat.idx, cacheStatus.cachePos, seat.idx);
			break;
		}
	}
	else if (!cacheStatus.existInCache) {
		myStat.accL1++;
		myStat.accVic++;
		myStat.accL2++;
		myStat.missL1++;
		myStat.missVic++;
		myStat.missL2++;

		L1[seat.idx].valid = 1;
		L1[seat.idx].tag = seat.tag;
		L1[seat.idx].data = myMem[adr];

		if (L1_Evict.is_evict) {
			// store in victim
			VICTIMlru(VIC_Evict.pos_way);
			VICTIM[VIC_Evict.pos_way].valid = 1;
			VICTIM[VIC_Evict.pos_way].tag = (L1_Evict.block.tag << 4) + seat.idx;
			VICTIM[VIC_Evict.pos_way].data = L1_Evict.block.data;

			if (VIC_Evict.is_evict) {
				// store in L2
				L2lru(L2_Evict.pos_set, L2_Evict.pos_way); 
				L2[L2_Evict.pos_set][L2_Evict.pos_way].valid = 1;
				L2[L2_Evict.pos_set][L2_Evict.pos_way].tag = (VIC_Evict.block.tag >> 4);
				L2[L2_Evict.pos_set][L2_Evict.pos_way].data = VIC_Evict.block.data;

			}
		}
	}
}

void cache::retStat()
{
	double L1_miss_rate, L2_miss_rate, VIC_miss_rate, AAT;
	//compute the stats here:

	int L1_hit = 1;
	int VIC_hit = 1;
	int L2_hit = 8;
	int penalty = 100;

	L1_miss_rate = static_cast<double>(myStat.missL1) / myStat.accL1;
	L2_miss_rate = static_cast<double>(myStat.missL2) / myStat.accL2;
	VIC_miss_rate = static_cast<double>(myStat.missVic) / myStat.accVic;
	AAT = L1_hit + L1_miss_rate * (VIC_hit + VIC_miss_rate * (L2_hit + L2_miss_rate * penalty)); 

	cout << setprecision(10) << "(" << L1_miss_rate << "," << L2_miss_rate << "," << AAT << ")" << endl;

}