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


        bool optGenUpdate(MemReq* req) {
          Address address = req.lineAddr;
          //other stuff here
          return false;
        }

        bool predictor(MemReq* req) {
          if(optGenUpdate(req)) {
            //fill in
          }
          else
          {
            //fill in
          }
        }


    public:
        // add member methods here, refer to repl_policies.h
        HawkeyeReplPolicy(uint32_t _numLines, uint32_t _maxRpv) : array(0), numLines(_numLines), maxRpv(_maxRpv) {
          array = gm_calloc<uint32_t>(numLines);
        }

        ~HawkeyeReplPolicy() {
          gm_free(array);
        }

        void update(uint32_t id, const MemReq* req) {
          if(id == justReplaced) {
            justReplaced = -1;
          }
          else {
            array[id] = 0;
          }
        }

        void replaced(uint32_t id) {
          array[id] = maxRpv - 1;
          justReplaced = id;
        }

        template <typename C> uint32_t rank(const MemReq* req, C cands) {
            while(true)
            {
              for (auto ci = cands.begin(); ci != cands.end(); ci.inc()) {
                  if(array[*ci] >= maxRpv)
                  {
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
