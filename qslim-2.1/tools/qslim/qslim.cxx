/************************************************************************

  QSlim command line program.  This provides a very simple interface to
  the underlying functionality.  Basically, it just reads in the input,
  simplifies it, and writes out the results.  It couldn't be simpler.

  Copyright (C) 1998 Michael Garland.  See "COPYING.txt" for details.
  
  $Id: qslim.cxx,v 1.10 2000/11/20 20:52:41 garland Exp $

 ************************************************************************/
#include "externQslim.h"

int main()
{

    Qslim_alg("/home/sato/Desktop/dp/qslim-2.1/tools/qslim/bones.obj","bones333.obj",333,3,1,1000);

    return 0;
}
