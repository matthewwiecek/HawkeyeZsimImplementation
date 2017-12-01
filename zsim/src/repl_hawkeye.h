#ifndef REPL_HAWKEYE_H_
#define REPL_HAWKEYE_H_

#include "repl_policies.h"


struct occVector_element {
  Address address;
  uint8_t entry;

  static int lastIndexOf(occVector_element* occVector, Address address, size_t size, size_t front, size_t numLines) {
    for (size_t i = front; i != abs((front+1)%size); i = abs((i-1)%size)) {
      if (occVector[i].address == address) {
        return i;
      } else if (occVector[i].entry >= numLines) {
        return -1;
      }
    }
    return -1;
  }
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
        hash<Address> addr_hash;

        occVector_element* occVector;
        uint32_t occVector_size;
        uint32_t occVector_front = 0;

        bool optGenUpdate(const MemReq* req) {
          Address address = req->lineAddr >> numOffsetBits;


          int lastIndex = occVector_element::lastIndexOf(occVector, address, occVector_size, occVector_front, numLines);
          if (lastIndex != -1) {
            for (int i = occVector_front; i != lastIndex; i = abs((i-1)%occVector_size)) {
              occVector[i].entry++;
            }
            return true;
          }

          occVector_front++;
          occVector_front = abs(occVector_front % occVector_size);

          occVector[occVector_front].entry = 0;
          occVector[occVector_front].address = address;

          return false;
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
        HawkeyeReplPolicy(uint32_t _numLines, uint32_t lineSize, uint32_t _maxRpv) : array(0), numLines(_numLines), maxRpv(_maxRpv), numOffsetBits(log2(lineSize/8)) {
          array = gm_calloc<uint32_t>(numLines);
          pc_array = gm_calloc<uint8_t>(pow(2, pcHashSize));
          occVector_size = 8*numLines;
          occVector = gm_calloc<occVector_element>(occVector_size);
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
