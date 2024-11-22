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
    lvpt(lvptSize, {0}),
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

bool LVPredictor::lookup(Addr pc, std::array<uint8_t, 8> *val)
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

    // Lookup the value annd copy it into passed in array
    *val = lvpt[lvpt_idx];

    DPRINTF(LVP, "Returning value {%d, %d, %d, %d, %d, %d, %d, %d} from the LVPT\n",
                (*val)[0], (*val)[1], (*val)[2], (*val)[3], (*val)[4], (*val)[5], (*val)[6], (*val)[7]);
    
    return predict;
}

void LVPredictor::update(Addr pc, bool correct, std::array<uint8_t, 8> val)
{
    // If the value was correct, just increment the LCT counter
    // Otherwise, update LVPT value and decrement counter
    unsigned lct_idx = getLCTIndex(pc);
    if(correct) {
        lct[lct_idx]++;

        DPRINTF(LVP, "Due to a correct prediction, LCT[%d] is being incremented\n", lct_idx);
    } else {
        unsigned lvpt_idx = getLVPTIndex(pc);
        lvpt[lvpt_idx] = val;
        lct[lct_idx]--;

        DPRINTF(LVP, "Due to an incorrect prediction, LVPT[%d] is being updated\n", lvpt_idx);
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