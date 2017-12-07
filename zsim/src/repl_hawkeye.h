#ifndef REPL_HAWKEYE_H_
#define REPL_HAWKEYE_H_

#include "repl_policies.h"
#include <mutex>

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

const int MAX_WAYS = 16;
const int occVector_size = 8*MAX_WAYS;
struct occVect {
	occVector_element element[occVector_size];
	uint32_t front = 0;
	std::mutex mtx;
};

// Hawkeye Replacement Policy
class HawkeyeReplPolicy : public ReplPolicy {
    protected:
        uint32_t* array;
        uint32_t numLines;
        const uint32_t maxRpv;
        int64_t justReplaced = -1;
        uint8_t* pc_array;
        const uint32_t predBits = 3;
        const uint32_t pcHashSize = 13;
				const unsigned int numOffsetBits;
				const unsigned int numIndexBits;
				const unsigned int totalNonTagBits;
				const unsigned int numWays;
        hash<Address> addr_hash;

        occVect* occVector;
        uint32_t occVector_size;

				bool _optGenUpdate(const MemReq* req, occVect& occVector) {
				lock_guard<std::mutex> lck(occVector.mtx);
				  bool toReturn = false;
				  Address address = req->lineAddr >> totalNonTagBits;

				  int lastIndex = occVector_element::lastIndexOf(occVector.element, address, occVector_size, occVector.front, numWays);
				  if (lastIndex != -1) {
					for (unsigned int i = modulo(occVector.front-1, occVector_size); i != modulo(lastIndex-1,occVector_size); i = modulo(i-1,occVector_size)) {
					  occVector.element[i].entry++;
					}
					toReturn = true;
				  }

				  occVector.element[occVector.front].entry = 0;
				  occVector.element[occVector.front].address = address;

				  occVector.front = modulo(occVector.front+1, occVector_size);

				  return toReturn;
				}

				bool optGenUpdate(const MemReq* req) {
					unsigned int line = (req->lineAddr >> numOffsetBits) & ((1<<numIndexBits)-1);
					return _optGenUpdate(req, occVector[line]);
				}

		        //True: Cache-friendly
		        //False: Cache-averse
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

		    public:
		        // add member methods here, refer to repl_policies.h
		        HawkeyeReplPolicy(uint32_t _numLines, uint32_t lineSize, uint32_t _maxRpv, uint32_t _numWays) : array(0), numLines(_numLines), maxRpv(_maxRpv), numOffsetBits(ceil(log2(lineSize/8))),
					numIndexBits(ceil(log2(numLines))), totalNonTagBits(numOffsetBits+numIndexBits), numWays(_numWays) {
		          array = gm_calloc<uint32_t>(numLines);
		          pc_array = gm_calloc<uint8_t>(pow(2, pcHashSize));
		          occVector = gm_calloc<occVect>(pow(2,numIndexBits));
		          //allocate address array
		        }

		        ~HawkeyeReplPolicy() {
		          gm_free(array);
		          gm_free(pc_array);
		          gm_free(occVector);
		        }

		        void update(uint32_t id, const MemReq* req) {
		          if (predictor(req)) {
		            array[id] = 0;
		          } else {
		            array[id] = maxRpv - 1;
		          }
		        }

		        void replaced(uint32_t id) {
		          //let update handle it, policy is the same
		        }

		        template <typename C> uint32_t rank(const MemReq* req, C cands) {
		            while(true) {
		              for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
		                  if(array[*ci] >= maxRpv) {
		                    return *ci;
		                  }
		              }
		              for(auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
		                array[*ci]++;
		              }
		          }
		        }

        DECL_RANK_BINDINGS;
};
#endif //REPL_HAWKEYE_H_
