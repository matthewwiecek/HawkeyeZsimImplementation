#include "memory_hierarchy.h"
#include <cstdint>
#include <cassert>
using namespace std;


bool optGenUpdate(const MemReq* req) {
	return false;
}

int main() {
	const int size = 10;
	MemReq req;
	uint64_t mem_addr[] = {1, 2, 3, 4, 1, 1, 2, 4, 3, 1};
	bool correct[] = {false, false, false, false, true, true, true, false, false, true};
	for(int i = 1; i < size; ++i) {
		req.lineAddr = mem_addr[i];
		assert(correct[i] == optGenUpdate(&req));
	}
	return 0;
}
