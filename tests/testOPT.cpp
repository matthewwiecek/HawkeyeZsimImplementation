#include "memory_hierarchy.h"
#include <cstdint>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;


/*bool optGenUpdate(const MemReq* req) {
	return false;
}*/

unsigned long modulo(long i, long m) {
	long result = i % m;
	if (result < 0) {
		result += m;
	}
	return result;
}

struct occVector_element {
  Address address = 0;
  uint8_t entry = 0;

  static int lastIndexOf(occVector_element* occVector, Address address, size_t size, size_t front, size_t numWays) {
    for (size_t i = front; i != modulo(front+1,size); i = modulo(i-1,size)) {
      if (occVector[i].entry >= numWays) {
        return -1;
      } else if (occVector[i].address == address) {
        return i;
      }
    }
    return -1;
  }
};

const int occVector_size = 16;
struct occVect {
	occVector_element element[occVector_size];
	uint32_t front = 0;
};

int numWays = 2;
const int numOffsetBits = 3;
occVect occVector[(long) pow(2,numOffsetBits)];


bool _optGenUpdate(const MemReq* req, occVect& occVector) {
  bool toReturn = false;
  Address address = req->lineAddr >> numOffsetBits;

  int lastIndex = occVector_element::lastIndexOf(occVector.element, address, occVector_size, occVector.front, numWays);
  if (lastIndex != -1) {
	  cout << "Last Index: " << lastIndex << " i != " << modulo(lastIndex-1, occVector_size) << endl;
	for (int i = modulo(occVector.front-1, occVector_size); i != modulo(lastIndex-1,occVector_size); i = modulo(i-1,occVector_size)) {
	  occVector.element[i].entry++;
	}
	toReturn = true;
  }
  
  occVector.element[occVector.front].entry = 0;
  occVector.element[occVector.front].address = address;

  occVector.front++;
  occVector.front = modulo(occVector.front, occVector_size);

  return toReturn;
}

bool optGenUpdate(const MemReq* req) {
	unsigned int line = req->lineAddr & ((1<<(numOffsetBits)-1);
	return _optGenUpdate(req, occVector[line]);
}

string dumpOPT(unsigned int line) {
	stringstream ss;
	for(int i = 0; i < occVector_size; ++i) {
		ss << occVector[line].element[i].address << ", ";
	}
	ss << "\n";
	for(int i = 0; i < occVector_size; ++i) {
		ss << (int) occVector[line].element[i].entry << ", ";
	}
	return ss.str();
}


int main() {
	const int size = 10;
	//const int size = 12;
	MemReq req;
	uint64_t orig_mem_addr[] = {1, 2, 3, 4, 1, 1, 2, 3, 4, 1};
	bool correct[] = {false, false, false, false, true, true, true, false, false, true};
	//uint64_t orig_mem_addr[] = {1, 2, 2, 3, 4, 5, 1, 6, 4, 5, 6, 3};
	//bool correct[] = {false, false, true, false, false, false, true, false, true, false, true, false};
	
	uint64_t mem_addr[size] = {0};
	for(int j = 0; j < pow(2,numOffsetBits); ++j) {
		for(int i = 0; i < size; ++i) {
			mem_addr[i] = orig_mem_addr[i] << numOffsetBits;
			mem_addr[i] = mem_addr[i] | j;
		}
		for(int i = 0; i < size; ++i) {
			req.lineAddr = mem_addr[i];
			bool result = optGenUpdate(&req);
			cout << "Expected: " << correct[i] << " Got: " << result << endl;
			//cout << "OPTVec: \n" << dumpOPT() << endl;
			assert(correct[i] == result);
		}
	}
	return 0;
}
