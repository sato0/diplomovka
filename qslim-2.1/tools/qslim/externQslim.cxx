#include "singleQ.h"


extern "C" void Qslim_alg(char *Ainput_file_name,char *Aoutput_file_name,int Aface_c,int Aplacement_policy, int Aquadric_weight,double Aboundary_weight)
{
  	QSLIM(Ainput_file_name,Aoutput_file_name, Aface_c, Aplacement_policy, Aquadric_weight, Aboundary_weight);
}

