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
  Address address;
  uint8_t entry;

  static int lastIndexOf(occVector_element* occVector, Address address, size_t size, size_t front, size_t numLines) {
    for (size_t i = front; i != modulo(front+1,size); i = modulo(i-1,size)) {
      if (occVector[i].entry >= numLines) {
        return -1;
      } else if (occVector[i].address == address) {
        return i;
      }
    }
    return -1;
  }
};

occVector_element occVector[20];
int occVector_size = 20;
int occVector_front = 0;
int numLines = 2;

bool optGenUpdate(const MemReq* req) {
  bool toReturn = false;
  //Address address = req->lineAddr >> numOffsetBits;
  Address address = req->lineAddr;

  int lastIndex = occVector_element::lastIndexOf(occVector, address, occVector_size, occVector_front, numLines);
  if (lastIndex != -1) {
	  cout << "Last Index: " << lastIndex << " i != " << modulo(lastIndex-1, occVector_size) << endl;
	for (int i = modulo(occVector_front-1, occVector_size); i != modulo(lastIndex-1,occVector_size); i = modulo(i-1,occVector_size)) {
	  occVector[i].entry++;
	}
	toReturn = true;
  }
  
  occVector[occVector_front].entry = 0;
  occVector[occVector_front].address = address;

  occVector_front++;
  occVector_front = modulo(occVector_front, occVector_size);

  return toReturn;
}

string dumpOPT() {
	stringstream ss;
	for(int i = 0; i < occVector_size; ++i) {
		ss << occVector[i].address << ", ";
	}
	ss << "\n";
	for(int i = 0; i < occVector_size; ++i) {
		ss << (int) occVector[i].entry << ", ";
	}
	return ss.str();
}


int main() {
	const int size = 10;
	//const int size = 12;
	MemReq req;
	int64_t mem_addr[] = {1, 2, 3, 4, 1, 1, 2, 3, 4, 1};
	bool correct[] = {false, false, false, false, true, true, true, false, false, true};
	//int64_t mem_addr[] = {1, 2, 2, 3, 4, 5, 1, 6, 4, 5, 6, 3};
	//bool correct[] = {false, false, true, false, false, false, true, false, true, false, true, false};
	for(int i = 0; i < size; ++i) {
		req.lineAddr = mem_addr[i];
		bool result = optGenUpdate(&req);
		cout << "Expected: " << correct[i] << " Got: " << result << endl;
		cout << "OPTVec: \n" << dumpOPT() << endl;
		assert(correct[i] == result);
	}
	return 0;
}
