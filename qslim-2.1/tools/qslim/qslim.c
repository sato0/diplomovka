extern "C" {

  #include "postgres.h"
  #include "fmgr.h"
  #include <stdio.h>
  #include <stdlib.h>

  #include <string.h>
  #include "catalog/pg_type.h"

  #include <unistd.h>
  #include <sys/stat.h>
  #include <stdbool.h>
  #include "utils/builtins.h"
  #include "externQslim.h"

  PG_MODULE_MAGIC;

    //PG_MODULE_MAGIC;

  void Qslim_alg( char *,char *,int ,int , int ,double );

  PG_FUNCTION_INFO_V1(qslim);

  Datum qslim(PG_FUNCTION_ARGS){

    char *Ainput_file_name = (PG_GETARG_CSTRING(0));
    char *Aoutput_file_name = (PG_GETARG_CSTRING(1));
    int Aface_c = PG_GETARG_INT32(2);
    int Aplacement_policy = PG_GETARG_INT32(3);
    int Aquadric_weight = PG_GETARG_INT32(4);
    double Aboundary_weight = PG_GETARG_FLOAT8(5);
    char *temp_str;
    char notice[500];
    sprintf(notice,  "before function");
    elog(NOTICE, notice);

    Qslim_alg(Ainput_file_name,Aoutput_file_name,Aface_c, Aplacement_policy, Aquadric_weight, Aboundary_weight);   
    
    sprintf(notice,  "after function");
    elog(NOTICE, notice);

    temp_str = "Qslim end.\n";
    PG_RETURN_TEXT_P(temp_str);
  }

}
 

 
 
 

 

