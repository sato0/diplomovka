/************************************************************************

  QSlim command line program.  This provides a very simple interface to
  the underlying functionality.  Basically, it just reads in the input,
  simplifies it, and writes out the results.  It couldn't be simpler.

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: qslim.cxx,v 1.10 2000/11/20 20:52:41 garland Exp $

 ************************************************************************/
#include <stdmix.h>
#include <mixio.h>
#include <MxTimer.h>
#include "qslim.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#else
#  include <getopt.h>
#endif



static void usage_error(char *msg=NULL)
{
    if( msg )  cerr << "Error: " << msg << endl;
    exit(1);
}

static ostream& vfcount(ostream& out, uint v, uint f)
{
    return out << "(" << v << "v/" << f << "f)";
}
//const char *Ainput_file_name,char *Aoutput_file_name
void startup_and_input(const char *Ainput_file_name,char *Aoutput_file_name,int Aface_c,int Aplacement_policy=3, int Aquadric_weight=1,bool Arun_quiet=true,double Aboundary_weight=1000)
{
    smf = new MxSMFReader;
    //tu by sa mohli spracovat vstupne argumenty namiesto process_cmdline

    int ival = Aplacement_policy;
    if( ival<0 || ival>3 ){
      placement_policy = 3;
	//usage_error("Illegal optimization policy.");
    }
    else  placement_policy = ival;

    ival = Aquadric_weight;
    if( ival<0 || ival>2 ){
      weighting_policy = 1;
	//usage_error("Illegal weighting policy.");
    }
    else weighting_policy = ival;
  
    boundary_weight = Aboundary_weight;
    face_target = Aface_c;
    output_filename = Aoutput_file_name;
    be_quiet = Arun_quiet;

    smf->unparsed_hook = unparsed_hook;
    m = new MxStdModel(100, 100);

    //nastavit cestu k suboru
    
	input_file(Ainput_file_name);
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   
   if( m->face_count() == 0 )
    {
	smf->read(cin, m);
    }
}

void QSLIM (const char *Ainput_file_name,char *Aoutput_file_name,int Aface_c,int Aplacement_policy=3, int Aquadric_weight=1,bool Arun_quiet=true,double Aboundary_weight=1000)
{
   /* const char *Ainput_file_name,
    char *Aoutput_file_name,
    int Aface_c,
    int Aplacement_policy=3, 
    int Aquadric_weight=1,bool Arun_quiet=true,
    double Aboundary_weight=1000
    */
    startup_and_input(Ainput_file_name,Aoutput_file_name,Aface_c,Aplacement_policy,Aquadric_weight=1, Arun_quiet=true, Aboundary_weight=1000);
    slim_init();
    slim->decimate(face_target);
    output_final_model();
    //cerr << "faces: " << slim->valid_faces  << endl << "vertex: " << slim->valid_verts << endl;
    slim_cleanup();
    // add print for output
}

int main()
{

    QSLIM("/home/sato/Desktop/dp/qslim-2.1/tools/qslim/bones.obj","bones200.obj",200);

    return 0;
}
