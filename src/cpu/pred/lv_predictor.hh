#ifndef __CPU_PRED_LV_PREDICTOR_HH__
#define __CPU_PRED_LV_PREDICTOR_HH__o

#include <vector>
#include <array>

#include "params/LVPredictor.hh"
#include "sim/sim_object.hh"
#include "base/sat_counter.hh"

namespace gem5 
{
    class LVPredictor : public SimObject 
    {
        public:
            /**
             * Default Constructor
             */
            LVPredictor(const LVPredictorParams &p);

            /**
             * Looks up the predicted value at the given pc addr.
             * @param pc The address of the loads pc
             * @param val A pointer to where the predicted value will be placed
             * @return Whether the value should be predicted or not 
             */
            bool lookup(Addr pc, std::array<uint8_t, 8> *val);

            /**
             * Updates the LVPT and LCT based on the actual outcome of the load
             * @param pc The address of the loads pc
             * @param correct Whether or not the value was predicted correctly by the LVPT
             * @param val Value returned by the actual load
             */
            void update(Addr pc, bool correct, std::array<uint8_t, 8> val);
        
        private:
            /**
            *  Returns the taken/not taken prediction given the value of the
            *  counter.
            *  @param count The value of the counter.
            *  @return The prediction based on the counter value.
            */
            inline bool getPrediction(uint8_t &count);

            /** Calculates the LVPT index based on the PC. */
            inline unsigned getLVPTIndex(Addr &PC);

            /** Calculates the LCT index based on the PC. */
            inline unsigned getLCTIndex(Addr &PC);

            /**
             * Amount to shift instruction by for indexing. This is set to 2
             * by default, as this is what the branch predictor uses
             */
            const unsigned instShiftAmt;

            /**
             * Number of entries in the LVPT
             */
            const unsigned lvptSize;

            /**
             * Bit mask for obtaining LVPT indexes
             */
            const unsigned lvptIndMask;

            /**
             * LVPT data structure, holds data arrays of size 8
             * (Since data can be up to 8 bytes long)
             */
            std::vector<std::array<uint8_t, 8>> lvpt;

            /**
             * Number of entries in the LCT
             */
            const unsigned lctSize;

            /**
             * Number of bits per LCT entry
             */
            const unsigned lctBits;

            /**
             * Bit mask for obtaining LCT indexes
             */
            const unsigned lctIndMask;

            /**
             * Vector of counters for lct, each is an lctBits sized
             * counter
             */
            std::vector<SatCounter8> lct;

             
    };
} // namespace gem5

#endif // __CPU_PRED_LV_PREDICTOR_HH__