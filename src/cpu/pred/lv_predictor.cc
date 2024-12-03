#include "cpu/pred/lv_predictor.hh"
#include "base/trace.hh"
#include "debug/LVP.hh"
#include "base/intmath.hh"

namespace gem5
{
LVPredictor::LVPredictor(const LVPredictorParams &params) : 
    SimObject(params),
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

bool LVPredictor::lookup(Addr pc, PacketPtr* packet)
{
    bool predict;

    // Get the index for the LCT
    unsigned lct_idx = getLCTIndex(pc);

    DPRINTF(LVP, "Looking up index %d in the LCT\n", lct_idx);

    // Lookup the counter value
    uint8_t count = lct[lct_idx];
    
    DPRINTF(LVP, "LCT value is %i\n", (int) count);

    // Get the prediction
    predict = getPrediction(count);
    if (predict) {
        DPRINTF(LVP, "LCT predicts that the LVPT value should be used\n");
        
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
    }
    
    return predict;
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