#include "cpu/pred/lv_predictor.hh"
#include "base/trace.hh"
#include "debug/LVP.hh"
#include "base/intmath.hh"

namespace gem5
{
CVU::CVU(const CVUParams &params) :
    SimObject(params),
    table(params.cvuSize, {.pc = 0, .addr = 0, .lru = 0, .valid = false}) {

    DPRINTF(LVP, "Created a CVU with size = %d\n", params.cvuSize);
}

bool CVU::lookup(Addr pc, Addr loadAddr) {
    DPRINTF(LVP, "Looking up address 0x%x for instruction with pc = 0x%x in the CVU\n", loadAddr, pc);

    // Find the location of the value in the table
    int idx = -1;
    for(int i = 0; i < table.size(); i++) {
        if(table[i].valid && table[i].addr == loadAddr && table[i].pc == pc) {
            idx = i;
            break;
        }
    }

    // If no value is found, return that. If a value was found, the LRU counts
    // must be modified
    if(idx == -1) {
        DPRINTF(LVP, "No CVU entry exists for address 0x%x\n", loadAddr);
        return false;
    } else {
        DPRINTF(LVP, "Found CVU entry for address 0x%x at index %d, updating LRU counts\n", loadAddr, idx);
        int old_lru = table[idx].lru;

        //Increment all valid lru values that are less than the old one
        for(int i = 0; i < table.size(); i++) {
            if(table[idx].valid && table[idx].lru < old_lru) {
                table[idx].lru++;
            }
        }

        //Update the accessed entry to be the most recently used
        table[idx].lru = 1;

        return true;
    }
}

void CVU::update(Addr pc, Addr loadAddr) {
    //If there is an empty spot in the table, we can put it in for free
    //Else we need to find a value to replace
    if(numEntries < table.size()) {
        int i;
        for(i = 0; i < table.size(); i++) {
            // Put it in the first non valid entry
            if(!table[i].valid) {
                table[i] = {.pc = pc, .addr = loadAddr, .lru = ++numEntries, .valid = true};
                
                DPRINTF(LVP, "Address 0x%x has been place at index %d in the cvu, which was empty\n", loadAddr, i);
                break;
            }
        }

        //There was a non-valid entry, so i should never have reached passed the end of the table
        assert(i < table.size());
    } else {
        //There should be a value with an LRU equal to the table size, it should be the replacement target
        int i;
        for(i = 0; i < table.size(); i++) {
            if(table[i].lru == table.size()) {
                DPRINTF(LVP, "Evicting address 0x%x from the CVU, replacing with address 0x%x\n", table[i].addr, loadAddr);

                table[i] = {.pc = pc, .addr = loadAddr, .lru = numEntries, .valid = true};
                break;
            }
        }

        //There should have been an entry with the maximum LRU value, so i should not have reached table.size()
        assert(i < table.size());
    }
}

void CVU::invalidate(Addr storeAddr) {
    //All values with this address must be invalidated (there could be multiple if two pcs are loading from
    //the same address)

    for(int i = 0; i < table.size(); i++) {
        if(table[i].addr == storeAddr) {
            DPRINTF(LVP, "Invalidating address 0x%x at index %d\n and fixing LRU values", storeAddr, i);
            table[i].valid = false;

            //Now the LRU values must be updated
            int old_lru = table[i].lru;
            table[i].lru = 0;
            for(int j = 0; j < table.size(); j++) {
                if(table[j].lru > old_lru) {
                    table[j].lru--;
                }
            }
        }
    }
}

LVPredictor::LVPredictor(const LVPredictorParams &params) : 
    SimObject(params),
    cvu(*params.cvu),
    instShiftAmt(params.instShiftAmt),
    lvptSize(params.LVPTSize),
    lvptIndMask(lvptSize - 1),
    lvpt(lvptSize, nullptr),
    lctSize(params.LCTSize),
    lctBits(params.LCTBits),
    lctIndMask(lctSize - 1),
    lct(lctSize, SatCounter8(lctBits))
{
    if (!isPowerOf2(lvptSize)) {
        fatal("Invalid LVPT size!\n");
    }

    if (!isPowerOf2(lctSize)) {
        fatal("Invalid LCT size!\n");
    }

    DPRINTF(LVP, "Created LVP with LVPT size = %d, LCT size = %d, LCT bits = %d\n", lvptSize, lctSize, lctBits);
}

LVPredictor::~LVPredictor() {
    for(int i = 0; i < lvptSize; i++) {
        if(lvpt[i]) {
            delete lvpt[i];
        }
    }
}

int LVPredictor::lookup(Addr pc, PacketPtr* packet)
{
    bool predict;
    bool constant;

    // Get the index for the LCT
    unsigned lct_idx = getLCTIndex(pc);

    DPRINTF(LVP, "Looking up index %d in the LCT\n", lct_idx);

    // Lookup the counter value
    uint8_t count = lct[lct_idx];
    
    DPRINTF(LVP, "LCT value is %i\n", (int) count);

    // Get the prediction
    predict = getPrediction(count);
    constant = getConst(count);
    if (predict) {
        DPRINTF(LVP, "LCT predicts that the LVPT value should be used\n");
        if(constant) {
            DPRINTF(LVP, "LCT thinks this is a constant value\n");
        }
    } else {
        DPRINTF(LVP, "LCT predicts that the LVPT value should not be used\n");
    }

    // Get the index for the LVPT
    unsigned lvpt_idx = getLVPTIndex(pc);

    DPRINTF(LVP, "Looking up index %d in the LVPT\n", lvpt_idx);

    // Lookup the value and copy it into passed in packet if it is valid 
    if(lvpt[lvpt_idx]) {
        *packet = new Packet(lvpt[lvpt_idx], false, true);
        const uint8_t* val = (*packet)->getConstPtr<uint8_t>();
        DPRINTF(LVP, "Returning value {%d, %d, %d, %d, %d, %d, %d, %d} from the LVPT\n",
                val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7]);
    } else {
        DPRINTF(LVP, "No valid entry in the LVPT for this index\n");
        // Make sure we dont predict in this case, as that would cause issues later
        predict = false;
    }
    
    if(predict) {
        if(constant) {
            return 2;
        } else {
            return 1;
        }
    } else {
        return 0;
    }
}

void LVPredictor::update(Addr pc, bool correct, PacketPtr packet) {
    // If the packet given is not valid, dont update anything
    if(!packet) {
        return;
    }
    // If the value was correct, increment the LCT counter and update the LVPT
    // Otherwise, update LVPT value and decrement counter
    unsigned lct_idx = getLCTIndex(pc);
    unsigned lvpt_idx = getLVPTIndex(pc);
    if(correct) {
        //Delete the old entry at this position and add a new one
        if(lvpt[lvpt_idx]) {
            delete lvpt[lvpt_idx];
        }
        lvpt[lvpt_idx] = new Packet(packet, false, true);

        lct[lct_idx]++;

        DPRINTF(LVP, "Due to a correct prediction, LCT[%d] is being incremented\n", lct_idx);
    } else {
        //Delete the old entry at this position and add a new one
        if(lvpt[lvpt_idx]) {
            delete lvpt[lvpt_idx];
        }
        lvpt[lvpt_idx] = new Packet(packet, false, true);
        
        lct[lct_idx]--;

        DPRINTF(LVP, "Due to an incorrect prediction, LVPT[%d] is being updated\n", lvpt_idx);
        DPRINTF(LVP, "LVPT[%d][0] = %d\n", lvpt_idx, lvpt[lvpt_idx] -> getConstPtr<uint8_t>()[0]);
        DPRINTF(LVP, "Due to an incorrect prediction, LCT[%d] is being decremented\n", lct_idx);
    }
}

inline bool
LVPredictor::getPrediction(uint8_t &count)
{
    // Get the MSB of the count (1 is predict, 0 is not predict) 
    return (count >> (lctBits - 1));
}

inline bool
LVPredictor::getConst(uint8_t &count) {
    // Only return true if the counter is at its max
    uint8_t max = 1;
    max = max << lctBits;
    return(count == (max - 1));
}

inline
unsigned
LVPredictor::getLVPTIndex(Addr &pc)
{
    return (pc >> instShiftAmt) & lvptIndMask;
}

inline
unsigned
LVPredictor::getLCTIndex(Addr &pc)
{
    return (pc >> instShiftAmt) & lctIndMask;
}

} //namespace gem5