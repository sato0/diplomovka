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


  #ifdef PG_MODULE_MAGIC
    PG_MODULE_MAGIC;
  #endif

//   bool fileExists( char* file) {
//     struct stat buf;
//     return (stat(file, &buf) == 0);
// }

  PG_FUNCTION_INFO_V1(merge2filesPG_func);

  Datum merge2filesPG_func(PG_FUNCTION_ARGS){


    char *file1 = text_to_cstring(PG_GETARG_TEXT_P_COPY(0));
    char *file2 = text_to_cstring(PG_GETARG_TEXT_P_COPY(1));
     char *temp_str = "undefined";
     char notice[500];
     char ch;
     int i = 0;
     int j = 0;
        sprintf(notice,  "Prebratie agmentov");
        elog(NOTICE, notice);

    FILE *fs1 = fopen(file1,"r");
    FILE *fs2 = fopen(file2,"r");
      sprintf(notice,  "Spojenie na subory.");
      elog(NOTICE, notice);
    if( fs1 == NULL){
        sprintf(notice,  "Neotvorilo 1. subor.");
        elog(NOTICE, notice);          
        temp_str =  file1;

      PG_RETURN_TEXT_P(temp_str);
    }
    if(fs2 == NULL){
        sprintf(notice,  "Neotvorilo 2. subor.");
        elog(NOTICE, notice);         
        temp_str = file2;

      PG_RETURN_TEXT_P(temp_str);
    }

    FILE *ft = fopen("result.obj","w");

    if( ft == NULL ){
      temp_str = " Unable open file for writing result.\n";
      PG_RETURN_TEXT_P(temp_str);
    }

    sprintf(notice,  "Malo by otvorit subory.");
    elog(NOTICE, notice);


    while( ( ch = fgetc(fs1) ) != EOF ){
      i++;      
      fputc(ch,ft);
      sprintf(notice,  "kopirujem znak. %d  %d",i,ch);
      elog(NOTICE, notice);
    }
     fclose(fs1);

    while( ( ch = fgetc(fs2) ) != EOF ){
      j++;
      fputc(ch,ft);
      sprintf(notice,  "kopirujem znak. %d  %d" ,j,ch);
      elog(NOTICE, notice);
    }
    fclose(fs2);

    fclose(ft);

    sprintf(notice,  "Malo by skopirovat subory.");
    elog(NOTICE, notice);

    
    
    temp_str = " Two files were merged into result.obj file successfully.\n";
     PG_RETURN_TEXT_P(temp_str);
  }


 

 
 
 

 

