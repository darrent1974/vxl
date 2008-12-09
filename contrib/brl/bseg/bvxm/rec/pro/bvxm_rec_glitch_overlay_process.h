// This is brl/bseg/bvxm/rec/pro/bvxm_rec_glitch_overlay_process.h
#ifndef bvxm_rec_glitch_overlay_process_h_
#define bvxm_rec_glitch_overlay_process_h_
//:
// \file
// \brief A class to generate a new probability map that extends glitch detection probability over to its effective region
//
// \author Ozge Can Ozcanli
// \date Dec 09, 2008
//
// \verbatim
//  Modifications
//   <none yet>
// \endverbatim

#include <vcl_string.h>
#include <bprb/bprb_process.h>

class bvxm_rec_glitch_overlay_process : public bprb_process
{
 public:

  bvxm_rec_glitch_overlay_process();

  //: Copy Constructor (no local data)
  bvxm_rec_glitch_overlay_process(const bvxm_rec_glitch_overlay_process& other): bprb_process(*static_cast<const bprb_process*>(&other)){};

  ~bvxm_rec_glitch_overlay_process(){};

  //: Clone the process
  virtual bvxm_rec_glitch_overlay_process* clone() const {return new bvxm_rec_glitch_overlay_process(*this);}

  vcl_string name(){return "bvxmRecGlitchOverlayProcess";}

  bool init() { return true; }
  bool execute();
  bool finish(){return true;}
};


#endif // bvxm_rec_glitch_overlay_process_h_
