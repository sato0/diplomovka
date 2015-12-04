
#include <stdio.h>
#include <stdlib.h>
 
 int merge2files(char file1[],char file2[]){
   
   FILE *fs1, *fs2, *ft;
   char ch;
   fs1 = fopen(file1,"r");
   fs2 = fopen(file2,"r");
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

 
  merge2files("lake_vertex.obj","lake_face.obj");
 
  return 0;
}