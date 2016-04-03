#include <stdmix.h>
#include <mixio.h>
#include <MxTimer.h>
#include "qslim.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#else
#  include <getopt.h>
#endif

//with no default args: QSLIM (input_file_name,output_file_name,face_count,3,1,1000)

extern "C" void QSLIM (char *Ainput_file_name,char *Aoutput_file_name,int Aface_c,int Aplacement_policy, int Aquadric_weight,double Aboundary_weight)
{
      //startup and input
   smf = new MxSMFReader;

    int ival = Aplacement_policy;
    if( ival<0 || ival>3 ){
      placement_policy = 3;
    }
    else  placement_policy = ival;

    ival = Aquadric_weight;
    if( ival<0 || ival>2 ){
      weighting_policy = 1;
    }
    else weighting_policy = ival;
  
    boundary_weight = Aboundary_weight;
    face_target = Aface_c;
    output_filename = Aoutput_file_name;
    be_quiet = true;

    smf->unparsed_hook = unparsed_hook;
    m = new MxStdModel(100, 100);

    
    input_file(Ainput_file_name);
   
   if( m->face_count() == 0 )
    {
    smf->read(cin, m);
    }
    //startup and input END

    slim_init();
    slim->decimate(face_target);
    output_final_model();
    //cerr << "faces: " << slim->valid_faces  << endl << "vertex: " << slim->valid_verts << endl;
   // slim_cleanup();
    // add print for output
}