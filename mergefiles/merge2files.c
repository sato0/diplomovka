
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>

  bool fileExists( char* file) {
    struct stat buf;
    return (stat(file, &buf) == 0);
  }
 
 int merge2files(char file1[],char file2[]){
   
   FILE *fs1, *fs2, *ft;
   char ch;
   fs1 = fopen(file1,"r");
   fs2 = fopen(file2,"r");


   if (fileExists(file1) && fileExists(file2)){
      printf("FILES EXIST!!!");
    }

    else{
      printf("FILES NOT EXIST!!!");
    }

   if( fs1 == NULL || fs2 == NULL ){
      printf("Unable open file.\n");
      return 1;
   }
   
   ft = fopen("result.obj","w");
 
   if( ft == NULL ){
      printf("Unable open file for writing result.\n");
      return 1;     
  }
  
   while( ( ch = fgetc(fs1) ) != EOF )
      fputc(ch,ft);
 
   while( ( ch = fgetc(fs2) ) != EOF )
      fputc(ch,ft);
 
   printf("Two files were merged into result.obj file successfully.\n");
 
   fclose(fs1);
   fclose(fs2);
   fclose(ft);
 
   return 0;
}
 
int main()
{

 
  merge2files("/home/sato/Desktop/dp/connectDP/lake2vertex.obj","/home/sato/Desktop/dp/connectDP/lake2face.obj");
 
  return 0;
}