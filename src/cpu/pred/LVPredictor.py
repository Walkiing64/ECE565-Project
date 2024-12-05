from m5.params import *
from m5.SimObject import SimObject

class CVU(SimObject) :
    type = 'CVU'
    cxx_header = "cpu/pred/lv_predictor.hh"
    cxx_class = "gem5::CVU"

    cvuSize = Param.Unsigned(16, "Number of entries is this CVU")
  
class LVPredictor(SimObject) :
    type = 'LVPredictor'
    cxx_header = "cpu/pred/lv_predictor.hh"
    cxx_class = "gem5::LVPredictor"
    
    LVPTSize = Param.Unsigned(1024, "Number of entries in the Load Value Predictor Table")
    LCTSize = Param.Unsigned(256, "Number of entries in the Load Configuration Table")
    LCTBits = Param.Unsigned(2, "Number of bits per entry in the Load Configureation Table")
    instShiftAmt = Param.Unsigned(2, "Number of bits to shift instructions by")
    cvu = Param.CVU(CVU(), "CVU attatched to this LVP")