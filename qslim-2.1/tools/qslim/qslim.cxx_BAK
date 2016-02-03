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



int main()
{
    double input_time, init_time, slim_time, output_time;

    // Process command line and read input model(s)
    //
    input_time = get_cpu_time();
    startup_and_input("bones.obj","bones1500.obj",1500);

    if(!be_quiet) vfcount(cerr, m->vert_count(), m->face_count()) << endl;

    // Initial simplification process.  Collect contractions and build heap.
    //
    cerr << "Start: "<< input_time;
    TIMING(init_time, slim_init());

    // Decimate model until target is reached
    //
    TIMING(slim_time, slim->decimate(face_target));

    if(!be_quiet) cerr << "+ Simplified model ";
    if(!be_quiet) vfcount(cerr, slim->valid_verts, slim->valid_faces) << endl;

    // Output the result
    //
    TIMING(output_time, output_final_model());


    cerr << slim->valid_faces << " " << init_time+slim_time << endl;
    

    slim_cleanup();

    return 0;
}
