#ifndef REPL_HAWKEYE_H_
#define REPL_HAWKEYE_H_

#include "repl_policies.h"

// Hawkeye Replacement Policy
class HawkeyeReplPolicy : public ReplPolicy {
    protected:
        // add class member variables here
        uint32_t* array;
        uint32_t numLines;
        const uint32_t maxRpv;
        int64_t justReplaced = -1;
        uint8_t* pc_array;
        const uint32_t predBits = 3;
        const uint32_t pcHashSize = 13;
        hash<Address> addr_hash;


        bool optGenUpdate(const MemReq* req) {
          //Address address = req.lineAddr;
          //other stuff here
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
        HawkeyeReplPolicy(uint32_t _numLines, uint32_t _maxRpv) : array(0), numLines(_numLines), maxRpv(_maxRpv) {
          array = gm_calloc<uint32_t>(numLines);
          pc_array = gm_calloc<uint8_t>(pow(2, pcHashSize));
        }

        ~HawkeyeReplPolicy() {
          gm_free(array);
          gm_free(pc_array);
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
