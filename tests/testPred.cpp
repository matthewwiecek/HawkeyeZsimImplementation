#include "memory_hierarchy.h"
#include <algorithm>
#include <math.h>
#include <cstdint>
#include <memory>
#include <cassert>
using namespace std;

const uint32_t predBits = 3;
const uint32_t pcHashSize = 13;
uint8_t pc_array[8192];
hash<Address> addr_hash;
MemReq friendly;
MemReq averse;

bool optGenUpdate(const MemReq* req) {
	if(req == &friendly) {
		return true;
	}
	return false;
}

bool predictor(const MemReq* req) {
	Address hashedPc = (Address) ((unsigned long) addr_hash(req->pc) % (unsigned long) pow(2, pcHashSize));
	if (optGenUpdate(req)) {
		if (pc_array[hashedPc] < pow(2, predBits) - 1) {
			pc_array[hashedPc]++;
		}
	} else {
		if (pc_array[hashedPc] > 0) {
			pc_array[hashedPc]--;
		}
	}

	if (pc_array[hashedPc] > pow(2, predBits - 1)) {
		return true;
	} else {
		return false;
	}
}

int main() {
	friendly.pc = 2;
	averse.pc = 3;
	for(int i = 0; i < 5; i++) {
		predictor(&friendly);
		predictor(&averse);
	}
	assert(predictor(&friendly) == true);
	assert(predictor(&averse) == false);
	return 0;
}
