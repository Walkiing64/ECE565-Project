#ifndef __CPU_PRED_LV_PREDICTOR_HH__
#define __CPU_PRED_LV_PREDICTOR_HH__

#include <vector>
#include <array>

#include "params/LVPredictor.hh"
#include "sim/sim_object.hh"
#include "base/sat_counter.hh"
#include "base/statistics.hh"
#include "base/stats/info.hh"
#include "base/stats/storage.hh"
#include "base/stats/types.hh"
#include "mem/packet.hh"

namespace gem5 
{
    class CVU : public SimObject
    {
        public:
            /**
             * Default Constructor
             */
            CVU(const CVUParams &p);

            /**
             * Looks up if the given pc and load address are in the table
             * @param pc The address of the instruction
             * @param loadAddr The address of the data being predicted
             */
            bool lookup(Addr pc, Addr loadAddr);

            /**
             * Puts the given pc and load address in the CVU, replacing a
             * value if there is no space
             * @param pc The address of the instruction
             * @param loadAddr The address of the data being predicted
             */
            void update(Addr pc, Addr loadAddr);

            /**
             * Invalidates any entries that match the address of the given store
             * @param storeAddr The address of the store that is doing the
             * invalidation
             */
            void invalidate(Addr storeAddr);
        
        private:
            /**
             * Structure for holding one CVU entry
             * @param pc Address of the load that placed this entry
             * @param addr Data address associated with this entry
             * @param lru Number used to determine which entry has been used
             * the least recently
             * @param valid True if this is a valid entry, false otherwise
             */
            struct CVUEntry {
                Addr pc;
                Addr addr;
                int lru;
                bool valid;
            };

            /**
             * Vector used to hold the CVU table
             */
            std::vector<CVUEntry> table;

            /**
             * number of valid entries currently in the table
             */
            int numEntries = 0;
    };

    class LVPredictor : public SimObject 
    {
        public:
            /**
             * Default Constructor
             */
            LVPredictor(const LVPredictorParams &p);

            /**
             * Default Destructor
             */
            ~LVPredictor();

            void regStats() override;
            /**
             * Looks up the predicted value at the given pc addr.
             * @param pc The address of the loads pc
             * @param packet A pointer to where the predicted values packet will be placed
             * @return Whether the value should be predicted or not
             *          0 = dont predict
             *          1 = predict
             *          2 = constant predict 
             */
            int lookup(Addr pc, PacketPtr* packet);

            /**
             * Updates the LVPT and LCT based on the actual outcome of the load
             * @param pc The address of the loads pc
             * @param correct Whether or not the value was predicted correctly by the LVPT
             * @param packet Packet returned by the actual load
             */
            void update(Addr pc, bool correct, PacketPtr packet);

            /**
             * The CVU connected to this load value predictor
             */
            CVU &cvu;
        
        private:

            statistics::Scalar LVPT_total;
            statistics::Scalar LVPT_hits;
            /**
            *  Returns the taken/not taken prediction given the value of the
            *  counter.
            *  @param count The value of the counter.
            *  @return The prediction based on the counter value.
            */
            inline bool getPrediction(uint8_t &count);

            /**
             * Returns constant/not constant for this prediction based on the
             * given counter valut
             * @param count The value of the counter
             * @return The constant prediction based on the counter value
             */
            inline bool getConst(uint8_t &count);

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
             * LVPT data structure, holds data response packet pointers
             */
            std::vector<PacketPtr> lvpt;

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
