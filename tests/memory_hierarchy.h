#include <cstdint>
using namespace std;

/* Memory request */
using Address = uint64_t;
struct MemReq {
    Address pc;
    Address lineAddr;
    uint32_t childId;
    uint64_t cycle; //cycle where request arrives at component

    //Used for race detection/sync

    //Requester id --- used for contention simulation
    uint32_t srcId;

    //Flags propagate across levels, though not to evictions
    //Some other things that can be indicated here: Demand vs prefetch accesses, TLB accesses, etc.
    enum Flag {
        IFETCH        = (1<<1), //For instruction fetches. Purely informative for now, does not imply NOEXCL (but ifetches should be marked NOEXCL)
        NOEXCL        = (1<<2), //Do not give back E on a GETS request (turns MESI protocol into MSI for this line). Used on e.g., ifetches and NUCA.
        NONINCLWB     = (1<<3), //This is a non-inclusive writeback. Do not assume that the line was in the lower level. Used on NUCA (BankDir).
        PUTX_KEEPEXCL = (1<<4), //Non-relinquishing PUTX. On a PUTX, maintain the requestor's E state instead of removing the sharer (i.e., this is a pure writeback)
        PREFETCH      = (1<<5), //Prefetch GETS access. Only set at level where prefetch is issued; handled early in MESICC
    };
    uint32_t flags;

    inline void set(Flag f) {flags |= f;}
    inline bool is (Flag f) const {return flags & f;}
};

/* Invalidation/downgrade request */
struct InvReq {
    Address lineAddr;
    // NOTE: writeback should start false, children pull it up to true
    bool* writeback;
    uint64_t cycle;
    uint32_t srcId;
};
