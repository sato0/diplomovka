/*
Copyright © 2014, 2015 Martin Kalivoda

This file is part of pg3angles-1.01.

pg3angles-1.01 is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

pg3angles-1.01 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with pg3angles-1.01.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "postgres.h"
#include "fmgr.h"


#include "math.h"
#include "utils/array.h"
#include <string.h>
#include "float.h"
#include "catalog/pg_type.h"



#ifndef M_PI
  #define M_PI 3.14159265358979323846
#endif

#define LOWPRCSN_EPSILON 1e-6

//Struktura 3 suradnic vektora
typedef struct
{
  float8 x;
  float8 y;
  float8 z;
} vektor3D;

PG_MODULE_MAGIC;

//Pomocne funkcie volane vo viacerych PostgreSQL/PostGIS zadefinovanych funkciach
float8 mymod (float8 delenec, float8 delitel)
{
  return delenec / delitel - ((delenec >= 0) ? floor(delenec / delitel) : ceil(delenec / delitel));
}

void parcialnederivacie(Datum trojuholnik, float8 *zx, float8 *zy)
{
  float8 x1, y1, z1, x2, y2, z2, x3, y3, z3;
  vektor3D v12, v13, w;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary;
  char filename[] = "postgis-2.2";
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  Datum vstup, medzivystup;
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(1)));
  x1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(1)));
  y1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(1)));
  z1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(2)));
  x2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(2)));
  y2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(2)));
  z2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(3)));
  x3 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(3)));
  y3 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, trojuholnik), Int32GetDatum(3)));
  z3 = DatumGetFloat8(medzivystup);
  
  v12.x = x2 - x1;
  v12.y = y2 - y1;
  v12.z = z2 - z1;
  v13.x = x3 - x1;
  v13.y = y3 - y1;
  v13.z = z3 - z1;
  
  w.x = v12.y * v13.z - v13.y * v12.z;
  w.y = v12.z * v13.x - v13.z * v12.x;
  w.z = v12.x * v13.y - v13.x * v12.y;
  
  *zx = -w.x / w.z;
  *zy = -w.y / w.z;  
}

void abcroviny(Datum kbod, Datum *sustroj, int poctroj, int typvahy, float8 *a, float8 *b, float8 *c)
{
  vektor3D *w, v12, v13;
  float8 xk, yk, x1, y1, z1, x2, y2, z2, x3, y3, z3, k, q, d12C, abc[3], velkost, *uhol, sumauhlov;
  Datum pomoc;
  int i, orientacia;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary;
  char filename[] = "postgis-2.2";
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  char notice[500];
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  
  pomoc =  DirectFunctionCall1(ST_X, kbod);
  xk = DatumGetFloat8(pomoc);
  pomoc =  DirectFunctionCall1(ST_Y, kbod);
  yk = DatumGetFloat8(pomoc);
  
  w = (vektor3D *) palloc(sizeof(vektor3D) * poctroj);
  uhol = (float8 *) palloc(sizeof(float8) * poctroj); //v pripade vahy stotoznenej s velkostou vrcholovych uhlov
    
  for (i = 0; i < poctroj; i++){
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(1)));
    x1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(1)));
    y1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(1)));
    z1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(2)));
    x2 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(2)));
    y2 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(2)));
    z2 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(3)));
    x3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(3)));
    y3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, sustroj[i]), Int32GetDatum(3)));
    z3 = DatumGetFloat8(pomoc);

    sprintf(notice, "xk je %f, yk je %f, x1 je %f, y1 je %f, z1 je %f, x2 je %f, y2 je %f, z2 je %f, x3 je %f, y3 je %f, z3 je %f", xk, yk, x1, y1, z1, x2, y2, z2, x3, y3, z3);
    elog(NOTICE, notice);
      
    if (fabs(x2 - x1) > FLT_EPSILON){ //smernica k je rozna od +-nekonecna
      k = (y2 - y1) / (x2 - x1);
      q = y1 - k * x1;
 
      d12C = k * x3 + q - y3;
          
      if (d12C > 0 && x1 < x2)
        orientacia = 1;
      else if (d12C > 0 && x1 > x2)
        orientacia = -1;
      else if (d12C < 0 && x1 < x2)
        orientacia = -1;
      else if (d12C < 0 && x1 > x2)
        orientacia = 1;
      sprintf(notice, "k je %f, q je %f, d12C je %f, orientacia je %d", k, q, d12C, orientacia);
      elog(NOTICE, notice);
    }
    else { //smernica k sa blizi k +- nekonecnu
      d12C = x3 - x1;
    
      if (d12C > 0 && y1 < y2)
        orientacia = 1;
      else if (d12C > 0 && y1 > y2)
        orientacia = -1;
      else if (d12C < 0 && y1 < y2)
        orientacia = -1;
      else if (d12C < 0 && y1 > y2)
        orientacia = 1;
      sprintf(notice, "d12C je %f, orientacia je %d", d12C, orientacia);
      elog(NOTICE, notice);
    }
      
    if (xk == x1 && yk == y1)
      if (orientacia == 1){
        v12.x = x2 - x1;
        v12.y = y2 - y1;
        v12.z = z2 - z1; 
        v13.x = x3 - x1;
        v13.y = y3 - y1; 
        v13.z = z3 - z1; 
        w[i].x = v12.y * v13.z - v13.y * v12.z;
        w[i].y = v12.z * v13.x - v13.z * v12.x;
        w[i].z = v12.x * v13.y - v13.x * v12.y; 
        uhol[i] = acos((v12.x * v13.x + v12.y * v13.y + v12.z * v13.z) / (sqrt(pow(v12.x, 2.0) + pow(v12.y, 2.0) + pow(v12.z, 2.0)) * sqrt(pow(v13.x, 2.0) + pow(v13.y, 2.0) + pow(v13.z, 2.0))));
        sprintf(notice, "moznost 1a, x12 je %f, y12 je %f, z12 je %f, x13 je %f, y13 je %f, z13 je %f, uhol je %f", v12.x, v12.y, v12.z, v13.x, v13.y, v13.z, uhol[i]);
        elog(NOTICE, notice);
      }
      else {
        v12.x = x3 - x1;
        v12.y = y3 - y1;
        v12.z = z3 - z1; 
        v13.x = x2 - x1;
        v13.y = y2 - y1; 
        v13.z = z2 - z1; 
        w[i].x = v12.y * v13.z - v13.y * v12.z;
        w[i].y = v12.z * v13.x - v13.z * v12.x;
        w[i].z = v12.x * v13.y - v13.x * v12.y;
        uhol[i] = acos((v12.x * v13.x + v12.y * v13.y + v12.z * v13.z) / (sqrt(pow(v12.x, 2.0) + pow(v12.y, 2.0) + pow(v12.z, 2.0)) * sqrt(pow(v13.x, 2.0) + pow(v13.y, 2.0) + pow(v13.z, 2.0))));
        sprintf(notice, "moznost 1b, x12 je %f, y12 je %f, z12 je %f, x13 je %f, y13 je %f, z13 je %f, uhol je %f", v12.x, v12.y, v12.z, v13.x, v13.y, v13.z, uhol[i]);
        elog(NOTICE, notice);
      }
    else if (xk == x2 && yk == y2)
      if (orientacia == 1){
        v12.x = x3 - x2;
        v12.y = y3 - y2;
        v12.z = z3 - z2; 
        v13.x = x1 - x2;
        v13.y = y1 - y2; 
        v13.z = z1 - z2; 
        w[i].x = v12.y * v13.z - v13.y * v12.z;
        w[i].y = v12.z * v13.x - v13.z * v12.x;
        w[i].z = v12.x * v13.y - v13.x * v12.y; 
        uhol[i] = acos((v12.x * v13.x + v12.y * v13.y + v12.z * v13.z) / (sqrt(pow(v12.x, 2.0) + pow(v12.y, 2.0) + pow(v12.z, 2.0)) * sqrt(pow(v13.x, 2.0) + pow(v13.y, 2.0) + pow(v13.z, 2.0))));
        sprintf(notice, "moznost 2a, x12 je %f, y12 je %f, z12 je %f, x13 je %f, y13 je %f, z13 je %f, uhol je %f", v12.x, v12.y, v12.z, v13.x, v13.y, v13.z, uhol[i]);
        elog(NOTICE, notice);
      }
      else {
        v12.x = x1 - x2;
        v12.y = y1 - y2;
        v12.z = z1 - z2; 
        v13.x = x3 - x2;
        v13.y = y3 - y2; 
        v13.z = z3 - z2; 
        w[i].x = v12.y * v13.z - v13.y * v12.z;
        w[i].y = v12.z * v13.x - v13.z * v12.x;
        w[i].z = v12.x * v13.y - v13.x * v12.y;
        uhol[i] = acos((v12.x * v13.x + v12.y * v13.y + v12.z * v13.z) / (sqrt(pow(v12.x, 2.0) + pow(v12.y, 2.0) + pow(v12.z, 2.0)) * sqrt(pow(v13.x, 2.0) + pow(v13.y, 2.0) + pow(v13.z, 2.0))));
        sprintf(notice, "moznost 2b, x12 je %f, y12 je %f, z12 je %f, x13 je %f, y13 je %f, z13 je %f, uhol je %f", v12.x, v12.y, v12.z, v13.x, v13.y, v13.z, uhol[i]);
        elog(NOTICE, notice);
      }
    else if (xk == x3 && yk == y3)
      if (orientacia == 1){
        v12.x = x1 - x3;
        v12.y = y1 - y3;
        v12.z = z1 - z3; 
        v13.x = x2 - x3;
        v13.y = y2 - y3; 
        v13.z = z2 - z3; 
        w[i].x = v12.y * v13.z - v13.y * v12.z;
        w[i].y = v12.z * v13.x - v13.z * v12.x;
        w[i].z = v12.x * v13.y - v13.x * v12.y;
        uhol[i] = acos((v12.x * v13.x + v12.y * v13.y + v12.z * v13.z) / (sqrt(pow(v12.x, 2.0) + pow(v12.y, 2.0) + pow(v12.z, 2.0)) * sqrt(pow(v13.x, 2.0) + pow(v13.y, 2.0) + pow(v13.z, 2.0))));
        sprintf(notice, "moznost 3a, x12 je %f, y12 je %f, z12 je %f, x13 je %f, y13 je %f, z13 je %f, uhol je %f", v12.x, v12.y, v12.z, v13.x, v13.y, v13.z, uhol[i]);
        elog(NOTICE, notice);
      }
      else {
        v12.x = x2 - x3;
        v12.y = y2 - y3;
        v12.z = z2 - z3; 
        v13.x = x1 - x3;
        v13.y = y1 - y3; 
        v13.z = z1 - z3; 
        w[i].x = v12.y * v13.z - v13.y * v12.z;
        w[i].y = v12.z * v13.x - v13.z * v12.x;
        w[i].z = v12.x * v13.y - v13.x * v12.y;
        uhol[i] = acos((v12.x * v13.x + v12.y * v13.y + v12.z * v13.z) / (sqrt(pow(v12.x, 2.0) + pow(v12.y, 2.0) + pow(v12.z, 2.0)) * sqrt(pow(v13.x, 2.0) + pow(v13.y, 2.0) + pow(v13.z, 2.0))));
        sprintf(notice, "moznost 3b, x12 je %f, y12 je %f, z12 je %f, x13 je %f, y13 je %f, z13 je %f, uhol je %f", v12.x, v12.y, v12.z, v13.x, v13.y, v13.z, uhol[i]);
        elog(NOTICE, notice);
      }   
  }
  
  if (typvahy == 1){ //Vahou je dvojnasobna velkost trojuholnikov
    abc[0] = 0;
    abc[1] = 0;
    abc[2] = 0;
    for (i = 0; i < poctroj; i++){
      abc[0] += w[i].x;
      abc[1] += w[i].y;
      abc[2] += w[i].z;
      sprintf(notice, "normala %d, x je %f, y je %f, z je %f", i, w[i].x, w[i].y, w[i].z);
      elog(NOTICE, notice);
    }
  }
  else if (typvahy == 2){ //Vahou su velkosti uhlov
    sumauhlov = 0;
    abc[0] = 0;
    abc[1] = 0;
    abc[2] = 0;
    for (i = 0; i < poctroj; i++){
      velkost = sqrt(pow(w[i].x, 2.0) + pow(w[i].y, 2.0) + pow(w[i].z, 2.0));
      w[i].x /= velkost;
      w[i].y /= velkost;
      w[i].z /= velkost;
      w[i].x *= uhol[i];
      w[i].y *= uhol[i];
      w[i].z *= uhol[i];
      sumauhlov += uhol[i];
      abc[0] += w[i].x;
      abc[1] += w[i].y;
      abc[2] += w[i].z;
      sprintf(notice, "uhol je %f, x je %f, y je %f, z je %f", uhol[i], w[i].x, w[i].y, w[i].z);
      elog(NOTICE, notice);
    }    
    abc[0] /= sumauhlov;
    abc[1] /= sumauhlov;
    abc[2] /= sumauhlov;
  }
  
  *a = abc[0];
  *b = abc[1];
  *c = abc[2];    
  
  sprintf(notice, "sumauhlov je %f, a je %f, b je %f, c je %f", sumauhlov, abc[0], abc[1], abc[2]);
  elog(NOTICE, notice);    
  
  pfree(uhol);
  uhol = (void *) NULL;
  pfree(w);
  w = (void *) NULL;  
}

//Makra informujuce o PostgreSQL/PostGIS funkciach zadefinovanych v tomto dokumente
PG_FUNCTION_INFO_V1(TIN_Area3D);
PG_FUNCTION_INFO_V1(TIN_Centroid);
PG_FUNCTION_INFO_V1(TIN_PartialDerivativesOfTriangle);
//PG_FUNCTION_INFO_V1(TIN_EditNeighboursInfo); Zrušená od verzie 1.01
//PG_FUNCTION_INFO_V1(TIN_ExtendM); Zrušená od verzie 1.01
PG_FUNCTION_INFO_V1(TIN_LinearZ);
PG_FUNCTION_INFO_V1(TIN_Flip); // Upravena do verzie 1.01
//PG_FUNCTION_INFO_V1(TIN_GetTid); Zrušená od verzie 1.01
PG_FUNCTION_INFO_V1(TIN_QuasiQuadraticZ);
PG_FUNCTION_INFO_V1(TIN_SlopeOfTriangle); 
PG_FUNCTION_INFO_V1(TIN_AspectOfTriangle);
PG_FUNCTION_INFO_V1(TIN_PartialDerivativesOnVertex);
PG_FUNCTION_INFO_V1(TIN_SlopeOnVertex);
PG_FUNCTION_INFO_V1(TIN_AspectOnVertex);
PG_FUNCTION_INFO_V1(TIN_QuasiQuadraticDerivatives);
PG_FUNCTION_INFO_V1(TIN_NormalsAngle);
PG_FUNCTION_INFO_V1(TIN_IsConvexHull);
//PG_FUNCTION_INFO_V1(TIN_GetNeighboursTids); Zrušená od verzie 1.01
PG_FUNCTION_INFO_V1(TIN_Contours);
PG_FUNCTION_INFO_V1(TIN_PartialDerivativesOnVertexAnglesWeight);
PG_FUNCTION_INFO_V1(TIN_Cubature);

/*
 * Samotne definicie funkcii
 */
   
 Datum TIN_Area3D(PG_FUNCTION_ARGS)
{
  float8 x1, y1, z1, x2, y2, z2, x3, y3, z3, rozloha;
  vektor3D v12, v13, w;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary, ST_AsText;
  char filename[] = "postgis-2.2";
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  char funcname6[] = "LWGEOM_asText";
  char *testwkt;
  int  slen;
  Datum vstup, medzivystup;
  float8 vystup;
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  ST_AsText = load_external_function(filename, funcname6, 1, NULL);
  
  vstup = PG_GETARG_DATUM(0);
  
  medzivystup = DirectFunctionCall1(ST_AsText, vstup);
  slen = VARSIZE(medzivystup) - VARHDRSZ + 1;
  testwkt = (char *) palloc (sizeof(char) * slen);
  memcpy(testwkt, VARDATA(medzivystup), slen);
  testwkt[slen - 1] = '\0'; 
  if (strstr(testwkt, "TRIANGLE Z") == NULL)
    elog(ERROR, "Incorrect input geometry type");
  
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(1)));
  x1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(1)));
  y1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(1)));
  z1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(2)));
  x2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(2)));
  y2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(2)));
  z2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(3)));
  x3 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(3)));
  y3 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(3)));
  z3 = DatumGetFloat8(medzivystup);
  
  v12.x = x2 - x1;
  v12.y = y2 - y1;
  v12.z = z2 - z1;
  v13.x = x3 - x1;
  v13.y = y3 - y1;
  v13.z = z3 - z1;
  
  w.x = v12.y * v13.z - v13.y * v12.z;
  w.y = v12.z * v13.x - v13.z * v12.x;
  w.z = v12.x * v13.y - v13.x * v12.y;
  
  rozloha = sqrt(pow(w.x, 2.0) + pow(w.y, 2.0) + pow(w.z, 2.0)) / 2;
  
  PG_RETURN_FLOAT8(rozloha);
}

 Datum TIN_Centroid(PG_FUNCTION_ARGS)
{
  float8 x1, y1, z1, x2, y2, z2, x3, y3, z3, xt, yt, zt;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary, ST_GeomFromText, ST_AsText;
  char filename[] = "postgis-2.2";
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  char funcname6[] = "LWGEOM_from_text";
  char funcname7[] = "LWGEOM_asText";
  char *testwkt;
  int  slen;
  Datum vstup, medzivystup, vystup;
  Oid Collation = PG_GET_COLLATION();
  char *wkt;
  text *wellknown;
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  ST_GeomFromText = load_external_function(filename, funcname6, 1, NULL);
  ST_AsText = load_external_function(filename, funcname7, 1, NULL);
  
  vstup = PG_GETARG_DATUM(0);
  
  medzivystup = DirectFunctionCall1(ST_AsText, vstup);
  slen = VARSIZE(medzivystup) - VARHDRSZ + 1;
  testwkt = (char *) palloc (sizeof(char) * slen);
  memcpy(testwkt, VARDATA(medzivystup), slen);
  testwkt[slen - 1] = '\0'; 
  if (strstr(testwkt, "TRIANGLE") == NULL)
    elog(ERROR, "Incorrect input geometry type");
  wkt = (char *) palloc (sizeof(char) * slen);
  
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(1)));
  x1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(1)));
  y1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(2)));
  x2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(2)));
  y2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(3)));
  x3 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(3)));
  y3 = DatumGetFloat8(medzivystup);
  
  if (strstr(testwkt, "TRIANGLE Z")){
    medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(1)));
    z1 = DatumGetFloat8(medzivystup);
    medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(2)));
    z2 = DatumGetFloat8(medzivystup);
    medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(3)));
    z3 = DatumGetFloat8(medzivystup);
    
    xt = (x1 + x2 + x3) / 3;
    yt = (y1 + y2 + y3) / 3;
    zt = (z1 + z2 + z3) / 3;
    
    sprintf(wkt, "POINT Z (%f %f %f)\0", xt, yt, zt);
    wellknown = (text *) palloc (VARHDRSZ + sizeof(char) * (strlen(wkt) + 1));
    SET_VARSIZE(wellknown, VARHDRSZ + sizeof(char) * (strlen(wkt) + 1));
    memcpy((void *) VARDATA(wellknown), wkt, sizeof(char) * (strlen(wkt) + 1));
  
    vystup = DirectFunctionCall1(ST_GeomFromText, PointerGetDatum(wellknown));
  }
  else {
    xt = (x1 + x2 + x3) / 3;
    yt = (y1 + y2 + y3) / 3;
    
    sprintf(wkt, "POINT (%f %f)\0", xt, yt);
    wellknown = (text *) palloc (VARHDRSZ + sizeof(char) * (strlen(wkt) + 1));
    SET_VARSIZE(wellknown, VARHDRSZ + sizeof(char) * (strlen(wkt) + 1));
    memcpy((void *) VARDATA(wellknown), wkt, sizeof(char) * (strlen(wkt) + 1));
  
    vystup = DirectFunctionCall1(ST_GeomFromText, PointerGetDatum(wellknown));
  }
 
  PG_RETURN_DATUM(vystup);
}

 Datum TIN_PartialDerivativesOfTriangle(PG_FUNCTION_ARGS)
{
  Datum vstup, medzivystup;
  ArrayType *vysledok;
  Oid element_type;
  Datum *elementy;
  int16 typlen;
  bool typbyval;
  char typalign;
  float8 zx, zy;
  PGFunction ST_AsText;
  char filename[] = "postgis-2.2";
  char funcname1[] = "LWGEOM_asText";
  char *testwkt;
  int  slen;
  
  ST_AsText = load_external_function(filename, funcname1, 1, NULL);
  
  vstup = PG_GETARG_DATUM(0);
  
  medzivystup = DirectFunctionCall1(ST_AsText, vstup);
  slen = VARSIZE(medzivystup) - VARHDRSZ + 1;
  testwkt = (char *) palloc (sizeof(char) * slen);
  memcpy(testwkt, VARDATA(medzivystup), slen);
  testwkt[slen - 1] = '\0'; 
  if (strstr(testwkt, "TRIANGLE Z") == NULL)
    elog(ERROR, "Incorrect input geometry type");
  
  parcialnederivacie(vstup, &zx, &zy);
  
  elementy = (Datum *) palloc(2 * sizeof(Datum));
  elementy[0] = Float8GetDatum(zx);
  elementy[1] = Float8GetDatum(zy);
   
  element_type = FLOAT8OID;  
  get_typlenbyvalalign(element_type, &typlen, &typbyval, &typalign);
  
  vysledok = construct_array(elementy, 2, element_type, typlen, typbyval, typalign);
  
  PG_RETURN_ARRAYTYPE_P(vysledok);
}

 Datum TIN_LinearZ(PG_FUNCTION_ARGS)
{
  float8 a, b, c, d, xr, yr, zr, x1, y1, z1, x2, y2, z2, x3, y3, z3, wvelkost;
  vektor3D v12, v13, w;
  text *typtext1, *typtext2;
  char *typ1, *typ2;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary, ST_GeometryType;
  char filename[] = "postgis-2.2";  
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  char funcname6[] = "geometry_geometrytype";
  Datum vstuptroj, vstupbod, pomoc;
  Oid Collation = PG_GET_COLLATION();
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  ST_GeometryType = load_external_function(filename, funcname6, 1, NULL);
  
  vstuptroj = PG_GETARG_DATUM(0);
  vstupbod = PG_GETARG_DATUM(1);
  
  pomoc = DirectFunctionCall1Coll(ST_GeometryType, Collation, vstuptroj);
  typtext1 = (text *) palloc (VARSIZE(pomoc));
  SET_VARSIZE(typtext1, VARSIZE(pomoc));
  typtext1 = DatumGetTextP(pomoc);
  typ1 = (char *) palloc (VARSIZE(typtext1) - VARHDRSZ);
  memcpy(typ1, VARDATA(typtext1), VARSIZE(typtext1) + 1 - VARHDRSZ);
  typ1[VARSIZE(typtext1) - VARHDRSZ] = '\0';
  
  pomoc = DirectFunctionCall1Coll(ST_GeometryType, Collation, vstupbod);
  typtext2 = (text *) palloc (VARSIZE(pomoc));
  SET_VARSIZE(typtext2, VARSIZE(pomoc));
  typtext2 = DatumGetTextP(pomoc);
  typ2 = (char *) palloc (VARSIZE(typtext2) + 1 - VARHDRSZ);
  memcpy(typ2, VARDATA(typtext2), VARSIZE(typtext2) - VARHDRSZ);
  typ2[VARSIZE(typtext2) - VARHDRSZ] = '\0';
  
  if (strcmp(typ1, "ST_Triangle") != 0)
    elog(ERROR, "Wrong geometry type of the 1st parameter");
  
  if (strcmp(typ2, "ST_Point") != 0)
    elog(ERROR, "Wrong geometry type of the 2nd parameter");
  
  pomoc = DirectFunctionCall1Coll(ST_X, Collation, vstupbod);
  xr = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_Y, Collation, vstupbod);
  yr = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_X, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(1)));
  x1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_Y, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(1)));
  y1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_Z, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(1)));
  z1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_X, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(2)));
  x2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_Y, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(2)));
  y2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_Z, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(2)));
  z2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_X, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(3)));
  x3 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_Y, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(3)));
  y3 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1Coll(ST_Z, Collation, DirectFunctionCall2Coll(ST_PointN, Collation, DirectFunctionCall1Coll(ST_Boundary, Collation, vstuptroj), Int32GetDatum(3)));
  z3 = DatumGetFloat8(pomoc);  
    
  v12.x = x2 - x1;    
  v12.y = y2 - y1;
  v12.z = z2 - z1;
  v13.x = x3 - x1;
  v13.y = y3 - y1;
  v13.z = z3 - z1;

  w.x = v12.y * v13.z - v13.y * v12.z;
  w.y = v12.z * v13.x - v13.z * v12.x;
  w.z = v12.x * v13.y - v13.x * v12.y;
  
  wvelkost =  sqrt(pow(w.x, 2.0) + pow(w.y, 2.0) + pow(w.z, 2.0));
  
  a = w.x / wvelkost;
  b = w.y / wvelkost;
  c = w.z / wvelkost;
  d = -1 * (a * x1 + b * y1 + c * z1);
  
  zr = -1 * (a * xr + b * yr + d) / c;
  
  PG_RETURN_FLOAT8(zr);
}

 Datum TIN_Flip (PG_FUNCTION_ARGS)
{
  /*
  Zavedenie premennych a inicializacia niektorych premennych
  */
  ArrayType *vystuptroj;
  Oid trojeltype;
  int16 o_typlen;
  bool o_typbyval;
  char o_typalign;
  float8 sgn1, sgn3, k, q, x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;
  Datum vstup1, vstup2, pomoc, trojuholniky[2], body[3], linetrojk[2];
  int32 j, l;
  int nekonecno;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary, ST_GeomFromText, ST_Relate;
  char filename[] = "postgis-2.2";  
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  char funcname6[] = "LWGEOM_from_text";
  char funcname7[] = "relate_pattern";                                             
  char wkt[300]; //Moze padat na konstatnej velkosti, ak ju nahodou prekroci!!!
  text *wellknown;
  text *pattern = (text *) palloc(VARHDRSZ + 10);
  char retazec[10] = "FF0FFF1F2";
  
  SET_VARSIZE(pattern, VARHDRSZ + 10);
  memcpy((void *) VARDATA(pattern), retazec, 10);
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  ST_GeomFromText = load_external_function(filename, funcname6, 1, NULL);
  ST_Relate = load_external_function(filename, funcname7, 1, NULL);
  
  vstup1 = PG_GETARG_DATUM(0);
  vstup2 = PG_GETARG_DATUM(1);
  
  /*
  Algoritmy na spravne urcenie 4 suradnic vrcholov trojuholnikov, ktorych spolocna
  strana sa bude preklapat
  */ 
  linetrojk[0] = DirectFunctionCall1(ST_Boundary, vstup1);
  linetrojk[1] = DirectFunctionCall1(ST_Boundary, vstup2);
  
  l = 0;
  for (j = 1; j <= 3; j++){
    body[0] = DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(j));
    if (DirectFunctionCall3(ST_Relate, body[0], linetrojk[0], PointerGetDatum(pattern))){
      pomoc = DirectFunctionCall1(ST_X, body[0]);
      x2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, body[0]);
      y2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Z, body[0]);
      z2 = DatumGetFloat8(pomoc);
      l = j;  
    } 
  } 
  
  if (l == 1){
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(2)));
    x1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(2)));
    y1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(2)));
    z1 = DatumGetFloat8(pomoc);  
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(3)));
    x3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(3)));
    y3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(3)));
    z3 = DatumGetFloat8(pomoc); 
  }
  else if (l == 2){
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(1)));
    x1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(1)));
    y1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(1)));
    z1 = DatumGetFloat8(pomoc);  
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(3)));
    x3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(3)));
    y3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(3)));
    z3 = DatumGetFloat8(pomoc); 
  }
  else {
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(1)));
    x1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(1)));
    y1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(1)));
    z1 = DatumGetFloat8(pomoc);  
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(2)));
    x3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(2)));
    y3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetrojk[1], Int32GetDatum(2)));
    z3 = DatumGetFloat8(pomoc);   
  }
  
  for (j = 1; j <= 3; j++){
    body[0] = DirectFunctionCall2(ST_PointN, linetrojk[0], Int32GetDatum(j)); 
    if (DirectFunctionCall3(ST_Relate, body[0], linetrojk[1], PointerGetDatum(pattern))){
      pomoc = DirectFunctionCall1(ST_X, body[0]);
      x4 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, body[0]);
      y4 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Z, body[0]);
      z4 = DatumGetFloat8(pomoc);  
    }
  } 
  
  /*
  Test, ci 4 body trojuholnikov, ktorych spolocna strana sa ma preklapat, tvoria
  konvexny 4-uholnik. Ak netvoria, program konci s chybovou hlaskou.
  */
  nekonecno = 0;
  if ((x4 - x2) != 0)
    k = (y4 - y2) / (x4 - x2);
  else
    nekonecno = 1;
  
  if (nekonecno == 0){
    q = y4 - k * x4;
    sgn1 = k * x1 + q - y1;
    if (fabs(sgn1) < FLT_EPSILON)
      sgn1 = 0;
    sgn3 = k * x3 + q - y3;
    if (fabs(sgn3) < FLT_EPSILON)
      sgn3 = 0;
  }
  else{
    sgn1 = x1 - x4;
    if (fabs(sgn1) < FLT_EPSILON)
      sgn1 = 0;
    sgn3 = x3 - x4;
    if (fabs(sgn3) < FLT_EPSILON)
      sgn3 = 0;
  }
  if (sgn1 == 0 || sgn3 == 0)
    elog(ERROR, "Boundary of the triangles is not convex");
  else if ((sgn1 > 0 && sgn3 > 0) || (sgn1 < 0 && sgn3 < 0))
    elog(ERROR, "Boundary of the triangles is not convex");  
  
  /*
  Zapis novych tvarov preklopenych trojuholnikov do pola trojuholnikov
  (zatial vsak bez M hodnot) a ziskanie potrebnych premennych z nich
  */  
  sprintf(wkt, "TRIANGLE Z ((%f %f %f, %f %f %f, %f %f %f, %f %f %f))", x1, y1, z1, x2, y2, z2, x4, y4, z4, x1, y1, z1);
  wellknown = (text *) palloc (VARHDRSZ + sizeof(char) * (strlen(wkt) + 1));
  SET_VARSIZE(wellknown, VARHDRSZ + sizeof(char) * (strlen(wkt) + 1));
  memcpy((void *) VARDATA(wellknown), wkt, sizeof(char) * (strlen(wkt) + 1));
  
  trojuholniky[0] = DirectFunctionCall1(ST_GeomFromText, PointerGetDatum(wellknown)); 
  
  pfree((void *) wellknown);
  wellknown = NULL;
  sprintf(wkt, "TRIANGLE Z ((%f %f %f, %f %f %f, %f %f %f, %f %f %f))", x3, y3, z3, x4, y4, z4, x2, y2, z2, x3, y3, z3);
  wellknown = (text *) palloc (VARHDRSZ + sizeof(char) * (strlen(wkt) + 1));
  SET_VARSIZE(wellknown, VARHDRSZ + sizeof(char) * (strlen(wkt) + 1));
  memcpy((void *) VARDATA(wellknown), wkt, sizeof(char) * (strlen(wkt) + 1));
  
  trojuholniky[1] = DirectFunctionCall1(ST_GeomFromText, PointerGetDatum(wellknown));      
  
  /*
  Tvorba vystupneho pola typu ArrayType z pola trojuholnikov typu Datum
  */
  trojeltype = get_fn_expr_argtype(fcinfo->flinfo, 0);
  get_typlenbyvalalign(trojeltype, &o_typlen, &o_typbyval, &o_typalign);
  vystuptroj = construct_array(trojuholniky, 2, trojeltype, o_typlen, o_typbyval, o_typalign);
  
  PG_RETURN_ARRAYTYPE_P(vystuptroj);
}

 Datum TIN_QuasiQuadraticZ(PG_FUNCTION_ARGS)
{
  ArrayType *susedia1, *susedia2, *susedia3;
  Oid trojeltype;
  int16 trojtyplen;
  bool trojtypbyval;
  char trojtypalign;
  bool *trojnulls;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary;
  char filename[] = "postgis-2.2";  
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  Datum kmenovy, pomoc, *sustroj[3], kbod, bodr;
  int poctroj[3], i;
  int32 l, m;
  vektor3D *w, v12, v13;
  float8 velkost, x1, y1, z1, x2, y2, z2, x3, y3, z3, d[3], xr, yr, zr, Pi[3], pi[3], Pt, Q[3], citatel1, citatel2, menovatel, k, q, *x0[3], *y0[3], a, b, c; 
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  
  kmenovy = PG_GETARG_DATUM(0);
  susedia1 = PG_GETARG_ARRAYTYPE_P(1);
  susedia2 = PG_GETARG_ARRAYTYPE_P(2);
  susedia3 = PG_GETARG_ARRAYTYPE_P(3);
  bodr = PG_GETARG_DATUM(4);
  
  trojeltype = ARR_ELEMTYPE(susedia1);
  get_typlenbyvalalign(trojeltype, &trojtyplen, &trojtypbyval, &trojtypalign);
  deconstruct_array(susedia1, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj[0], &trojnulls, &poctroj[0]);
  deconstruct_array(susedia2, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj[1], &trojnulls, &poctroj[1]);
  deconstruct_array(susedia3, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj[2], &trojnulls, &poctroj[2]);
  
  pomoc = DirectFunctionCall1(ST_X, bodr);
  xr = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Y, bodr);
  yr = DatumGetFloat8(pomoc);
  
  for(m = 0; m < 3; m++){
    kbod = DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(m + 1));
    abcroviny(kbod, sustroj[m], poctroj[m], 1, &a, &b, &c);
    velkost = sqrt(pow(a, 2) + pow(b, 2) + pow(c, 2));
    a /= velkost;
    b /= velkost;
    c /= velkost;
    d[m] = -1 * (a * DatumGetFloat8(DirectFunctionCall1(ST_X, kbod)) + b * DatumGetFloat8(DirectFunctionCall1(ST_Y, kbod)) + c * DatumGetFloat8(DirectFunctionCall1(ST_Z, kbod)));
    Pi[m] = -1 * (xr * a + yr * b + d[m]) / c;
  }
  
  for (l = 0; l < 3; l++){
    if (l == 0){
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
      x1 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
      y1 = DatumGetFloat8(pomoc);  
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
      x2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
      y2 = DatumGetFloat8(pomoc); 
    }
    else if (l == 1){
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
      x1 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
      y1 = DatumGetFloat8(pomoc);  
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
      x2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
      y2 = DatumGetFloat8(pomoc); 
    }
    else if (l == 2){
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
      x1 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
      y1 = DatumGetFloat8(pomoc);  
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
      x2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
      y2 = DatumGetFloat8(pomoc); 
    }
    
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(l + 1)));
    x3 = DatumGetFloat8(pomoc); //x3 predstavuje suradnicu x l-teho bodu kmenoveho trojuholnika
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(l + 1)));
    y3 = DatumGetFloat8(pomoc); //y3 predstavuje suradnicu y l-teho bodu kmenoveho trojuholnika

    if ((x2 - x1) != 0){ //smernica k je rozna od +-nekonecna
      k = (y2 - y1) / (x2 - x1);
      q = y1 - k * x1;
    
      if (k != 0){ //smernica k je rozna od 0 a od +- nekonecna
        x0[l] = (float8 *) palloc(sizeof(float8));
        x0[l][0] = ((y3 - q) / k) - x3;
        y0[l] = (float8 *) palloc(sizeof(float8));
        y0[l][0] = (k * x3 + q) - y3;
        
        pi[l] = (1 + cos(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0])) / 2;
        Q[l] = pow(sin(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0]), 2.0);
      }
      else { //smernica k je rovna 0
        x0[l] = (void *) NULL;
        y0[l] = (float8 *) palloc(sizeof(float8));
        y0[l][0] = q - y3;
        
        pi[l] = (1 + cos(((yr - y3) * M_PI) / y0[l][0])) / 2; 
        Q[l] = pow(sin(((yr - y3) * M_PI) / y0[l][0]), 2.0);
      }
    }
    else { //smernica k sa blizi k +- nekonecnu
      x0[l] = (float8 *) palloc(sizeof(float8));
      x0[l][0] = x1 - x3;
      y0[l] = NULL; 
      
      pi[l] = (1 + cos(((xr - x3) * M_PI) / x0[l][0])) / 2; 
      Q[l] = pow(sin(((xr - x3) * M_PI) / x0[l][0]), 2.0);   
    }
  }
  
  citatel1 = 0;
  citatel2 = 0;
  menovatel = 0;
  for (i = 0; i < 3; i++){
    citatel1 += Pi[i] * pi[i];
    citatel2 += Q[i];
    menovatel += pi[i];
  }
  
  citatel1 /= menovatel;
  citatel2 /= 2.25;  
  
  pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
  x1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
  y1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
  z1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
  x2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
  y2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
  z2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
  x3 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
  y3 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
  z3 = DatumGetFloat8(pomoc);  
    
  v12.x = x2 - x1;    
  v12.y = y2 - y1;
  v12.z = z2 - z1;
  v13.x = x3 - x1;
  v13.y = y3 - y1;
  v13.z = z3 - z1;

  w = (vektor3D *) palloc(sizeof(vektor3D));
  w[0].x = v12.y * v13.z - v13.y * v12.z;
  w[0].y = v12.z * v13.x - v13.z * v12.x;
  w[0].z = v12.x * v13.y - v13.x * v12.y;
  
  velkost =  sqrt(pow(w[0].x, 2.0) + pow(w[0].y, 2.0) + pow(w[0].z, 2.0));
  
  w[0].x /= velkost;
  w[0].y /= velkost;
  w[0].z /= velkost;
  d[0] = -1 * (w[0].x * x1 + w[0].y * y1 + w[0].z * z1);
  
  Pt = -1 * (w[0].x * xr + w[0].y * yr + d[0]) / w[0].z;
  
  zr = (citatel1 + Pt * citatel2) / (1 + citatel2);
  
  PG_RETURN_FLOAT8(zr);
}

 Datum TIN_SlopeOfTriangle(PG_FUNCTION_ARGS)
{
  float8 zx, zy, sklon;
  Datum vstup;
  
  vstup = PG_GETARG_DATUM(0);
  
  parcialnederivacie(vstup, &zx, &zy);
  sklon = atan(sqrt(zx * zx + zy * zy)) * 180 / M_PI;
  
  PG_RETURN_FLOAT8(sklon);
}

 Datum TIN_AspectOfTriangle(PG_FUNCTION_ARGS)
{
  float8 zx, zy, orientacia;
  Datum vstup;
  
  vstup = PG_GETARG_DATUM(0);
  
  parcialnederivacie(vstup, &zx, &zy);
  orientacia = 180 + atan2(-zx, -zy) * 180 / M_PI;
  
  PG_RETURN_FLOAT8(orientacia);
}

 Datum TIN_PartialDerivativesOnVertex(PG_FUNCTION_ARGS)
{
  ArrayType *susedia, *vysledok;
  Oid trojeltype, o_eltype;
  int16 trojtyplen, o_typlen;
  bool trojtypbyval, o_typbyval;
  char trojtypalign, o_typalign;
  bool *trojnulls;
  Datum *sustroj, kbod, *elementy;
  int poctroj;
  float8 zx, zy, a, b, c; 
  
  kbod = PG_GETARG_DATUM(0);
  susedia = PG_GETARG_ARRAYTYPE_P(1);
  
  trojeltype = ARR_ELEMTYPE(susedia);
  get_typlenbyvalalign(trojeltype, &trojtyplen, &trojtypbyval, &trojtypalign);
  deconstruct_array(susedia, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj, &trojnulls, &poctroj);
  
  abcroviny(kbod, sustroj, poctroj, 1, &a, &b, &c);
  
  zx = - a / c;
  zy = - b / c;
  
  elementy = (Datum *) palloc(2 * sizeof(Datum));
  elementy[0] = Float8GetDatum(zx);
  elementy[1] = Float8GetDatum(zy);
   
  o_eltype = FLOAT8OID;  
  get_typlenbyvalalign(o_eltype, &o_typlen, &o_typbyval, &o_typalign);
  
  vysledok = construct_array(elementy, 2, o_eltype, o_typlen, o_typbyval, o_typalign);
  
  PG_RETURN_ARRAYTYPE_P(vysledok);
}

 Datum TIN_SlopeOnVertex(PG_FUNCTION_ARGS)
{
  ArrayType *susedia;
  Oid trojeltype;
  int16 trojtyplen;
  bool trojtypbyval;
  char trojtypalign;
  bool *trojnulls;
  Datum *sustroj, kbod;
  int poctroj;
  float8 zx, zy, a, b, c, sklon; 
  
  kbod = PG_GETARG_DATUM(0);
  susedia = PG_GETARG_ARRAYTYPE_P(1);
  
  trojeltype = ARR_ELEMTYPE(susedia);
  get_typlenbyvalalign(trojeltype, &trojtyplen, &trojtypbyval, &trojtypalign);
  deconstruct_array(susedia, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj, &trojnulls, &poctroj);
  
  abcroviny(kbod, sustroj, poctroj, 1, &a, &b, &c);
  
  zx = - a / c;
  zy = - b / c;
  
  sklon = atan(sqrt(zx * zx + zy * zy)) * 180 / M_PI;
  
  PG_RETURN_FLOAT8(sklon); 
}

 Datum TIN_AspectOnVertex(PG_FUNCTION_ARGS)
{
  ArrayType *susedia;
  Oid trojeltype;
  int16 trojtyplen;
  bool trojtypbyval;
  char trojtypalign;
  bool *trojnulls;
  Datum *sustroj, kbod;
  int poctroj;
  float8 zx, zy, a, b, c, orientacia; 
  
  kbod = PG_GETARG_DATUM(0);
  susedia = PG_GETARG_ARRAYTYPE_P(1);
  
  trojeltype = ARR_ELEMTYPE(susedia);
  get_typlenbyvalalign(trojeltype, &trojtyplen, &trojtypbyval, &trojtypalign);
  deconstruct_array(susedia, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj, &trojnulls, &poctroj);
  
  abcroviny(kbod, sustroj, poctroj, 1, &a, &b, &c);
  
  zx = - a / c;
  zy = - b / c;
  
  orientacia = 180 + atan2(-zx, -zy) * 180 / M_PI;
  
  PG_RETURN_FLOAT8(orientacia); 
}

 Datum TIN_QuasiQuadraticDerivatives(PG_FUNCTION_ARGS)
{
  ArrayType *susedia1, *susedia2, *susedia3, *vysledok;
  Oid trojeltype, o_eltype;
  int16 trojtyplen, o_typlen;
  bool trojtypbyval, o_typbyval;
  char trojtypalign, o_typalign;
  bool *trojnulls;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary;
  char filename[] = "postgis-2.2";  
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  Datum kmenovy, pomoc, *sustroj[3], kbod, bodr, *elementy;
  int poctroj[3], i;
  int32 l, m;
  vektor3D *w, v12, v13;
  float8 velkost, x1, y1, z1, x2, y2, z2, x3, y3, z3, d[3], xr, yr, Pi[3], pi[3], Pt, Qpom[3], Q, k, q, *x0[3], *y0[3], a, b, c;
  float8 dx_Pi[3], dy_Pi[3], dx_pi[3], dy_pi[3], dx_Q, dy_Q, dx_Pt, dy_Pt, dx_sin[3], dy_sin[3], dx_P, dy_P, cit1, cit2, cit3, cit4, men1, cit5, cit6, cit7, cit8, men2;  
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  
  kmenovy = PG_GETARG_DATUM(0);
  susedia1 = PG_GETARG_ARRAYTYPE_P(1);
  susedia2 = PG_GETARG_ARRAYTYPE_P(2);
  susedia3 = PG_GETARG_ARRAYTYPE_P(3);
  bodr = PG_GETARG_DATUM(4);
  
  trojeltype = ARR_ELEMTYPE(susedia1);
  get_typlenbyvalalign(trojeltype, &trojtyplen, &trojtypbyval, &trojtypalign);
  deconstruct_array(susedia1, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj[0], &trojnulls, &poctroj[0]);
  deconstruct_array(susedia2, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj[1], &trojnulls, &poctroj[1]);
  deconstruct_array(susedia3, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj[2], &trojnulls, &poctroj[2]);
  
  pomoc = DirectFunctionCall1(ST_X, bodr);
  xr = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Y, bodr);
  yr = DatumGetFloat8(pomoc);
  
  // Vypocet P1, P2, P3
  for(m = 0; m < 3; m++){
    kbod = DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(m + 1));
    abcroviny(kbod, sustroj[m], poctroj[m], 1, &a, &b, &c);
    velkost = sqrt(pow(a, 2) + pow(b, 2) + pow(c, 2));
    a /= velkost;
    b /= velkost;
    c /= velkost;
    d[m] = -1 * (a * DatumGetFloat8(DirectFunctionCall1(ST_X, kbod)) + b * DatumGetFloat8(DirectFunctionCall1(ST_Y, kbod)) + c * DatumGetFloat8(DirectFunctionCall1(ST_Z, kbod)));
    Pi[m] = -1 * (xr * a + yr * b + d[m]) / c;
    dx_Pi[m] = -1 * (a / c);
    dy_Pi[m] = -1 * (b / c);
  }
  
  // Vypocet p1, p2, p3, Q, pomocnych derivacii sinusov pre derivaciu Q
  Q = 0;
  for (l = 0; l < 3; l++){
    if (l == 0){
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
      x1 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
      y1 = DatumGetFloat8(pomoc);  
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
      x2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
      y2 = DatumGetFloat8(pomoc); 
    }
    else if (l == 1){
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
      x1 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
      y1 = DatumGetFloat8(pomoc);  
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
      x2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
      y2 = DatumGetFloat8(pomoc); 
    }
    else if (l == 2){
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
      x1 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
      y1 = DatumGetFloat8(pomoc);  
      pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
      x2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
      y2 = DatumGetFloat8(pomoc); 
    }
    
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(l + 1)));
    x3 = DatumGetFloat8(pomoc); //x3 predstavuje suradnicu x l-teho bodu kmenoveho trojuholnika
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(l + 1)));
    y3 = DatumGetFloat8(pomoc); //y3 predstavuje suradnicu y l-teho bodu kmenoveho trojuholnika

    if ((x2 - x1) != 0){ //smernica k je rozna od +-nekonecna
      k = (y2 - y1) / (x2 - x1);
      q = y1 - k * x1;
    
      if (k != 0){ //smernica k je rozna od 0 a od +- nekonecna
        x0[l] = (float8 *) palloc(sizeof(float8));
        x0[l][0] = ((y3 - q) / k) - x3;
        y0[l] = (float8 *) palloc(sizeof(float8));
        y0[l][0] = (k * x3 + q) - y3;
        
        pi[l] = (1 + cos(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0])) / 2;
        Qpom[l] = pow(sin(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0]), 2.0);
        dx_sin[l] = (2 * M_PI * sin(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0]) * cos(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0])) / (2.25 * x0[l][0]);
        dy_sin[l] = (2 * M_PI * sin(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0]) * cos(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0])) / (2.25 * y0[l][0]);
        dx_pi[l] = -1 * ((M_PI * sin(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0])) / (2 * x0[l][0]));
        dy_pi[l] = -1 * ((M_PI * sin(((xr - x3) * M_PI) / x0[l][0] + ((yr - y3) * M_PI) / y0[l][0])) / (2 * y0[l][0]));
      }
      else { //smernica k je rovna 0
        x0[l] = (void *) NULL;
        y0[l] = (float8 *) palloc(sizeof(float8));
        y0[l][0] = q - y3;
        
        pi[l] = (1 + cos(((yr - y3) * M_PI) / y0[l][0])) / 2; 
        Qpom[l] = pow(sin(((yr - y3) * M_PI) / y0[l][0]), 2.0);
        dx_sin[l] = 0;
        dy_sin[l] = (2 * M_PI * sin(((yr - y3) * M_PI) / y0[l][0]) * cos(((yr - y3) * M_PI) / y0[l][0])) / (2.25 * y0[l][0]);
        dx_pi[l] = 0;
        dy_pi[l] = -1 * ((M_PI * sin(((yr - y3) * M_PI) / y0[l][0])) / (2 * y0[l][0]));
      }
    }
    else { //smernica k sa blizi k +- nekonecnu
      x0[l] = (float8 *) palloc(sizeof(float8));
      x0[l][0] = x1 - x3;
      y0[l] = NULL; 
      
      pi[l] = (1 + cos(((xr - x3) * M_PI) / x0[l][0])) / 2; 
      Qpom[l] = pow(sin(((xr - x3) * M_PI) / x0[l][0]), 2.0);
      dx_sin[l] = (2 * M_PI * sin(((xr - x3) * M_PI) / x0[l][0]) * cos(((xr - x3) * M_PI) / x0[l][0]) / (2.25 * x0[l][0]));
      dy_sin[l] = 0;
      dx_pi[l] = -1 * ((M_PI * sin(((xr - x3) * M_PI) / x0[l][0])) / (2 * x0[l][0]));
      dy_pi[l] = 0;
    }
    Q += Qpom[l];
  }
  Q /= 2.25;  
  
  // Vypocet Pt
  pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
  x1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
  y1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(1)));
  z1 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
  x2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
  y2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(2)));
  z2 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
  x3 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
  y3 = DatumGetFloat8(pomoc);
  pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, kmenovy), Int32GetDatum(3)));
  z3 = DatumGetFloat8(pomoc);  
    
  v12.x = x2 - x1;    
  v12.y = y2 - y1;
  v12.z = z2 - z1;
  v13.x = x3 - x1;
  v13.y = y3 - y1;
  v13.z = z3 - z1;

  w = (vektor3D *) palloc(sizeof(vektor3D));
  w[0].x = v12.y * v13.z - v13.y * v12.z;
  w[0].y = v12.z * v13.x - v13.z * v12.x;
  w[0].z = v12.x * v13.y - v13.x * v12.y;
  
  velkost =  sqrt(pow(w[0].x, 2.0) + pow(w[0].y, 2.0) + pow(w[0].z, 2.0));
  
  w[0].x /= velkost;
  w[0].y /= velkost;
  w[0].z /= velkost;
  d[0] = -1 * (w[0].x * x1 + w[0].y * y1 + w[0].z * z1);
  
  Pt = -1 * (w[0].x * xr + w[0].y * yr + d[0]) / w[0].z;
  dx_Pt = -1 * (w[0].x / w[0].z);
  dy_Pt = -1 * (w[0].y / w[0].z); 
  
  // Vypocet dx_Q a dy_Q
  dx_Q = dx_sin[0] + dx_sin[1] + dx_sin[2];
  dy_Q = dy_sin[0] + dy_sin[1] + dy_sin[2];
  
  // Vypocet dx_P
  cit1 = dx_Pi[0] * pi[0] + Pi[0] * dx_pi[0] + dx_Pi[1] * pi[1] + Pi[1] * dx_pi[1] + dx_Pi[2] * pi[2] + Pi[2] * dx_pi[2];
  cit2 = pi[0] + pi[0] * Q + pi[1] + pi[1] * Q + pi[2] + pi[2] * Q;
  cit3 = Pi[0] * pi[0] + Pi[1] * pi[1] + Pi[3] * pi[3];
  cit4 = dx_pi[0] + dx_pi[0] * Q + pi[0] * dx_Q + dx_pi[1] + dx_pi[1] * Q + pi[1] * dx_Q + dx_pi[2] + dx_pi[2] * Q + pi[2] * dx_Q;
  cit5 = dx_Pt * Q + Pt * dx_Q;
  cit6 = 1 + Q;
  cit7 = Pt * Q;
  cit8 = dx_Q;
  men1 = pi[0] + pi[0] * Q + pi[1] + pi[1] * Q + pi[2] + pi[2] * Q;
  men2 = 1 + Q; 
  
  dx_P = (cit1 * cit2 - cit3 * cit4) / pow(men1, 2.0) + (cit5 * cit6 - cit7 * cit8) / pow(men2, 2.0);
  
  // Vypocet dy_P
  cit1 = dy_Pi[0] * pi[0] + Pi[0] * dy_pi[0] + dy_Pi[1] * pi[1] + Pi[1] * dy_pi[1] + dy_Pi[2] * pi[2] + Pi[2] * dy_pi[2];
  cit4 = dy_pi[0] + dy_pi[0] * Q + pi[0] * dy_Q + dy_pi[1] + dy_pi[1] * Q + pi[1] * dy_Q + dy_pi[2] + dy_pi[2] * Q + pi[2] * dy_Q;
  cit5 = dy_Pt * Q + Pt * dy_Q;
  cit8 = dy_Q; 
  
  dy_P = (cit1 * cit2 - cit3 * cit4) / pow(men1, 2.0) + (cit5 * cit6 - cit7 * cit8) / pow(men2, 2.0);
  
  elementy = (Datum *) palloc(2 * sizeof(Datum));
  elementy[0] = Float8GetDatum(dx_P);
  elementy[1] = Float8GetDatum(dy_P);
   
  o_eltype = FLOAT8OID;  
  get_typlenbyvalalign(o_eltype, &o_typlen, &o_typbyval, &o_typalign);
  
  vysledok = construct_array(elementy, 2, o_eltype, o_typlen, o_typbyval, o_typalign);
  
  PG_RETURN_ARRAYTYPE_P(vysledok);
}

 Datum TIN_NormalsAngle(PG_FUNCTION_ARGS)
{
  ArrayType *linvstup, *kkvstup;
  Oid f8eltype;
  int16 f8typlen;
  bool f8typbyval;
  char f8typalign;
  bool *linnulls, *kknulls;
  Datum *linpd, *kkpd;
  int numlinpd, numkkpd;
  float8 uhol, linx, liny, linz, kkx, kky, kkz, vellin, velkk;
  vektor3D skalsucin;
  
  linvstup = PG_GETARG_ARRAYTYPE_P(0);
  kkvstup = PG_GETARG_ARRAYTYPE_P(1);
  
  f8eltype = ARR_ELEMTYPE(linvstup);
  get_typlenbyvalalign(f8eltype, &f8typlen, &f8typbyval, &f8typalign);
  deconstruct_array(linvstup, f8eltype, f8typlen, f8typbyval, f8typalign, &linpd, &linnulls, &numlinpd);
  f8eltype = ARR_ELEMTYPE(kkvstup);
  get_typlenbyvalalign(f8eltype, &f8typlen, &f8typbyval, &f8typalign);
  deconstruct_array(kkvstup, f8eltype, f8typlen, f8typbyval, f8typalign, &kkpd, &kknulls, &numkkpd);
  
  if (numlinpd < 2 || numlinpd > 3)
    elog(ERROR, "Incorrect number of items in the first input array");
  if (numkkpd < 2 || numkkpd > 3)
    elog(ERROR, "Incorrect number of items in the second input array");
  
  if (numlinpd == 2){
    linx = -1 * DatumGetFloat8(linpd[0]);
    liny = -1 * DatumGetFloat8(linpd[1]);
    linz = 1;
  }
  else {
    linx = DatumGetFloat8(linpd[0]);
    liny = DatumGetFloat8(linpd[1]);
    linz = DatumGetFloat8(linpd[2]);
  }
  
  if (numkkpd == 2){
    kkx = -1 * DatumGetFloat8(kkpd[0]);
    kky = -1 * DatumGetFloat8(kkpd[1]);
    kkz = 1;
  }
  else {
    kkx = DatumGetFloat8(kkpd[0]);
    kky = DatumGetFloat8(kkpd[1]);
    kkz = DatumGetFloat8(kkpd[2]);
  }
  
  vellin = sqrt(pow(linx, 2.0) + pow(liny, 2.0) + pow(linz, 2.0));
  velkk = sqrt(pow(kkx, 2.0) + pow(kky, 2.0) + pow(kkz, 2.0));
  
  skalsucin.x = linx * kkx;
  skalsucin.y = liny * kky;
  skalsucin.z = linz * kkz;
  
  uhol = acos((skalsucin.x + skalsucin.y + skalsucin.z) / (vellin * velkk)) * 180 / M_PI;
  
  PG_RETURN_FLOAT8(uhol);   
}

 Datum TIN_IsConvexHull(PG_FUNCTION_ARGS)
{
  ArrayType *vystup;
  Oid o_eltype;
  int16 o_typlen;
  bool o_typbyval;
  char o_typalign;
  Datum troj1, troj2, linetroj1, linetroj2, bod, pomoc, *pole;
  int32 j, l, nekonecno;
  float8 x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, k, q, sgn1, sgn3;
  bool isconv = true;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary, ST_Relate;
  char filename[] = "postgis-2.2";  
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  char funcname6[] = "relate_pattern";
  char wkt[300]; //Moze padat na konstatnej velkosti, ak ju nahodou prekroci!!!
  text *wellknown;
  text *pattern = (text *) palloc(VARHDRSZ + 10);
  text *pattern2 = (text *) palloc(VARHDRSZ + 10);
  char retazec[10] = "1F*FFF*F*";
  char retazec2[10] = "FF0FFF1F2";
  
  SET_VARSIZE(pattern, VARHDRSZ + 10);
  memcpy((void *) VARDATA(pattern), retazec, 10);
  SET_VARSIZE(pattern2, VARHDRSZ + 10);
  memcpy((void *) VARDATA(pattern2), retazec2, 10);
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  ST_Relate = load_external_function(filename, funcname6, 1, NULL);
  
  troj1 = PG_GETARG_DATUM(0);
  troj2 = PG_GETARG_DATUM(1);
  
  linetroj1 = DirectFunctionCall1(ST_Boundary, troj1);
  linetroj2 = DirectFunctionCall1(ST_Boundary, troj2);
  
  if (!(DirectFunctionCall3(ST_Relate, linetroj1, linetroj2, PointerGetDatum(pattern))))
    elog(ERROR, "Input triangles are not neighbours");  
  
  l = 0;
  for (j = 1; j <= 3; j++){
    bod = DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(j));
    if (DirectFunctionCall3(ST_Relate, bod, linetroj1, PointerGetDatum(pattern2))){
      pomoc = DirectFunctionCall1(ST_X, bod);
      x2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, bod);
      y2 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Z, bod);
      z2 = DatumGetFloat8(pomoc);
      l = j;  
    } 
  } 
  
  if (l == 1){
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(2)));
    x1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(2)));
    y1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(2)));
    z1 = DatumGetFloat8(pomoc);  
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(3)));
    x3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(3)));
    y3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(3)));
    z3 = DatumGetFloat8(pomoc); 
  }
  else if (l == 2){
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(1)));
    x1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(1)));
    y1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(1)));
    z1 = DatumGetFloat8(pomoc);  
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(3)));
    x3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(3)));
    y3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(3)));
    z3 = DatumGetFloat8(pomoc); 
  }
  else {
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(1)));
    x1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(1)));
    y1 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(1)));
    z1 = DatumGetFloat8(pomoc);  
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(2)));
    x3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(2)));
    y3 = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, linetroj2, Int32GetDatum(2)));
    z3 = DatumGetFloat8(pomoc);   
  }
  
  for (j = 1; j <= 3; j++){
    bod = DirectFunctionCall2(ST_PointN, linetroj1, Int32GetDatum(j)); 
    if (DirectFunctionCall3(ST_Relate, bod, linetroj2, PointerGetDatum(pattern2))){
      pomoc = DirectFunctionCall1(ST_X, bod);
      x4 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Y, bod);
      y4 = DatumGetFloat8(pomoc);
      pomoc = DirectFunctionCall1(ST_Z, bod);
      z4 = DatumGetFloat8(pomoc);  
    }
  }
  
  nekonecno = 0;
  if ((x4 - x2) != 0)
    k = (y4 - y2) / (x4 - x2);
  else
    nekonecno = 1;
  
  if (nekonecno == 0){
    q = y4 - k * x4;    
    sgn1 = k * x1 + q - y1;
    if (fabs(sgn1) < FLT_EPSILON)
      sgn1 = 0;
    sgn3 = k * x3 + q - y3;
    if (fabs(sgn3) < FLT_EPSILON)
      sgn3 = 0;
  }
  else{
    sgn1 = x1 - x4;
    if (fabs(sgn1) < FLT_EPSILON)
      sgn1 = 0;
    sgn3 = x3 - x4;
    if (fabs(sgn3) < FLT_EPSILON)
      sgn3 = 0;
  }
  if (sgn1 == 0 || sgn3 == 0)
    isconv = false;
  else if ((sgn1 > 0 && sgn3 > 0) || (sgn1 < 0 && sgn3 < 0))
    isconv = false;
  
  PG_RETURN_BOOL(isconv);
}

 Datum TIN_Contours(PG_FUNCTION_ARGS)
{
  ArrayType *vysledok;
  Oid o_eltype;
  int16 o_typlen;
  bool o_typbyval, test, jemaxmin, sort;
  char o_typalign;
  vektor3D b[3], *d, *h;
  float8 rozostup, dlzka, k, q, *medzilahlez, gmin, gmax, vzdialenost, x1, y1, x2, y2, dskrmnt, A, B, C, Px, Py, modulo, frac;
  int32 i, j, l, n, pocetvrst, nastrane, nekonecno, *kontrolnepole, special, pocvrststr, aktual, najmensi;
  Datum vstup, pomoc, **polebodov, *vrstevnice, linia;
  PGFunction ST_X, ST_Y, ST_Z, ST_PointN, ST_Boundary, ST_MakePoint, ST_MakeLine, ST_AsText, ST_Line_Interpolate_Point;
  char filename[] = "postgis-2.2";
  char funcname1[] = "LWGEOM_x_point";
  char funcname2[] = "LWGEOM_y_point";
  char funcname3[] = "LWGEOM_z_point";
  char funcname4[] = "LWGEOM_pointn_linestring";
  char funcname5[] = "boundary";
  char funcname6[] = "LWGEOM_makepoint";
  char funcname7[] = "LWGEOM_makeline";
  char funcname8[] = "LWGEOM_asText";
  char funcname9[] = "LWGEOM_line_interpolate_point";
  char *wkt;
  char notice[500];
  int32 velpola, presiel;
  
  ST_X = load_external_function(filename, funcname1, 1, NULL);
  ST_Y = load_external_function(filename, funcname2, 1, NULL);
  ST_Z = load_external_function(filename, funcname3, 1, NULL);
  ST_PointN = load_external_function(filename, funcname4, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname5, 1, NULL);
  ST_MakePoint = load_external_function(filename, funcname6, 1, NULL);
  ST_MakeLine = load_external_function(filename, funcname7, 1, NULL); 
  ST_AsText = load_external_function(filename, funcname8, 1, NULL);
  ST_Line_Interpolate_Point = load_external_function(filename, funcname9, 1, NULL); 
  
  vstup = PG_GETARG_DATUM(0);
  rozostup = PG_GETARG_FLOAT8(1);
  
  for (i = 0; i < 3; i++){
    pomoc = DirectFunctionCall1(ST_X, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(i + 1)));
    b[i].x = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Y, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(i + 1)));
    b[i].y = DatumGetFloat8(pomoc);
    pomoc = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(i + 1)));
    b[i].z = DatumGetFloat8(pomoc);
  }
  
  gmin = b[0].z;
  gmax = b[0].z;
  for (i = 1; i < 3; i++){
    if (b[i].z < gmin)
      gmin = b[i].z;
    if (b[i].z > gmax)
      gmax = b[i].z;
  }
  
  special = -1;
  if ((b[0].z == b[1].z) && (fabs(fmod(b[0].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[0].z, rozostup)) - rozostup) < FLT_EPSILON))
    special = 1; //vysky prveho a druheho bodu su totozne a ma cez ne viest vrstevnica 
  else if ((b[1].z == b[2].z)  && (fabs(fmod(b[1].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[1].z, rozostup)) - rozostup) < FLT_EPSILON))
    special = 2; //vysky prveho a tretieho bodu su totozne a ma cez ne viest vrstevnica
  else if ((b[2].z == b[0].z)  && (fabs(fmod(b[2].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[2].z, rozostup) - rozostup)) < FLT_EPSILON))
    special = 3; //vysky druheho a tretieho bodu su totozne a ma cez ne viest vrstevnica
  if (fabs(gmax - gmin) < FLT_EPSILON)
    special = 0; //vsetky 3 body maju totoznu vysku (cez trojuholnik nebude viest ziadna vrstevnica)
  
  pocetvrst = abs((int32)(gmax / rozostup) - (int32)(gmin / rozostup));
  sprintf(notice, "pocetvrst = %d", pocetvrst);
  elog(NOTICE, notice);
  if ((special == 1) && (b[0].z == gmin)){
    pocetvrst++;
  }
  else if ((special == 2) && (b[1].z == gmin)){
    pocetvrst++;
  }
  else if ((special == 3) && (b[2].z == gmin)){
    pocetvrst++;
  }
  
  sprintf(notice, "gmax/roz = %f, gmin/roz = %f, pocetvrst = %d, %d, %d", gmax/rozostup, gmin/rozostup, pocetvrst, (int32) (gmax/rozostup), (int32) (gmin/rozostup));
  elog(NOTICE, notice);

  aktual = 0;
  if (special != 0){
    if (pocetvrst > 0){
      polebodov = (Datum **) palloc (sizeof(Datum *) * pocetvrst * 2);
      for (i = 0; i < pocetvrst * 2; i++)
        polebodov[i] = (void *) NULL;
      
      //Vypocty pre liniu tvorenu prvym a druhym bodom trojuholnika  
      if (special == 1){
        polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
        polebodov[aktual][0] = DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[0].x), Float8GetDatum(b[0].y), Float8GetDatum(b[0].z));
        aktual++;
        polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
        polebodov[aktual][0] = DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[1].x), Float8GetDatum(b[1].y), Float8GetDatum(b[1].z));
        aktual++;  
        sprintf(notice, "Vrstevnica prechadza cez stranu 1,2");
        elog(NOTICE, notice);
      }
      else {
        pocvrststr = abs((int32)(b[0].z / rozostup) - (int32)(b[1].z / rozostup)); 
        
        sprintf(notice, "pocvrststr pred testami je %d", pocvrststr);
        elog(NOTICE, notice); 
        jemaxmin = false;
        if ((fabs(fmod(b[0].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[0].z, rozostup)) - rozostup) < FLT_EPSILON) && (b[0].z == gmax)){
          pocvrststr--;
          jemaxmin = true;
        }
        if ((fabs(fmod(b[1].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[1].z, rozostup)) - rozostup) < FLT_EPSILON) && (b[1].z == gmax)){
          pocvrststr--;
          jemaxmin = true;
        }
        
        sprintf(notice, "pocvrststr po testoch je %d", pocvrststr);
        elog(NOTICE, notice);
        
        if (fabs(b[0].z) < fabs(b[1].z))
          linia = DirectFunctionCall2(ST_MakeLine, DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[1].x), Float8GetDatum(b[1].y), Float8GetDatum(b[1].z)), DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[0].x), Float8GetDatum(b[0].y), Float8GetDatum(b[0].z)));
        else
          linia = DirectFunctionCall2(ST_MakeLine, DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[0].x), Float8GetDatum(b[0].y), Float8GetDatum(b[0].z)), DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[1].x), Float8GetDatum(b[1].y), Float8GetDatum(b[1].z)));                                
        
        if (pocvrststr > 0){
          if (fabs(b[0].z) > fabs(b[1].z)){ // Prvy bod je vyssi ako druhy
            if (jemaxmin){
              for (i = 1; i <= pocvrststr; i++){
                frac = (i * rozostup + fmod(fabs(b[0].z), rozostup)) / fabs(b[0].z - b[1].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "robim 1. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[0].z, rozostup));
                elog(NOTICE, notice);
              }
            }
            else {
              for (i = 0; i < pocvrststr; i++){
                frac = (i * rozostup + fmod(fabs(b[0].z), rozostup)) / fabs(b[0].z - b[1].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "robim 2. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[0].z, rozostup));
                elog(NOTICE, notice);
              }
            }
          }
          else { // Prvy bod je nizsi ako druhy            
            if (jemaxmin){
              for (i = 1; i <= pocvrststr; i++){
                //frac = (i * rozostup + rozostup - fmod(b[0].z, rozostup)) / fabs(b[0].z - b[1].z);
                frac = (i * rozostup + fmod(fabs(b[1].z), rozostup)) / fabs(b[1].z - b[0].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "frac je %lf", frac);
                elog(NOTICE, notice);
                sprintf(notice, "robim 3. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[0].z, rozostup));
                elog(NOTICE, notice);
              }
            }
            else {
              for (i = 0; i < pocvrststr; i++){
                //frac = (i * rozostup + rozostup - fmod(b[0].z, rozostup)) / fabs(b[0].z - b[1].z);
                frac = (i * rozostup + fmod(fabs(b[1].z), rozostup)) / fabs(b[1].z - b[0].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "frac je %lf", frac);
                elog(NOTICE, notice);
                sprintf(notice, "robim 4. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[0].z, rozostup));
                elog(NOTICE, notice);
              }
            }  
          }
        }
      }
      sprintf(notice, "aktual je %d", aktual);
      elog(NOTICE, notice);
      
      //Vypocty pre liniu tvorenu druhym a tretim bodom trojuholnika
      if (special == 2){
        polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
        polebodov[aktual][0] = DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[1].x), Float8GetDatum(b[1].y), Float8GetDatum(b[1].z));
        aktual++;
        polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
        polebodov[aktual][0] = DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[2].x), Float8GetDatum(b[2].y), Float8GetDatum(b[2].z));
        aktual++;
        sprintf(notice, "Vrstevnica prechadza cez stranu 2,3");
        elog(NOTICE, notice);  
      }
      else { 
        pocvrststr = abs((int32)(b[1].z / rozostup) - (int32)(b[2].z / rozostup)); 
        
        sprintf(notice, "pocvrststr pred testami je %d", pocvrststr);
        elog(NOTICE, notice); 
        jemaxmin = false;
        if ((fabs(fmod(b[1].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[1].z, rozostup)) - rozostup) < FLT_EPSILON) && (b[1].z == gmax)){
          pocvrststr--;
          jemaxmin = true;
        }
        if ((fabs(fmod(b[2].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[2].z, rozostup)) - rozostup) < FLT_EPSILON) && (b[2].z == gmax)){
          pocvrststr--;
          jemaxmin = true;
        } 
        
        sprintf(notice, "pocvrststr po testoch je %d", pocvrststr);
        elog(NOTICE, notice);
        
        if (fabs(b[1].z) < fabs(b[2].z))
          linia = DirectFunctionCall2(ST_MakeLine, DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[2].x), Float8GetDatum(b[2].y), Float8GetDatum(b[2].z)), DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[1].x), Float8GetDatum(b[1].y), Float8GetDatum(b[1].z)));
        else
          linia = DirectFunctionCall2(ST_MakeLine, DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[1].x), Float8GetDatum(b[1].y), Float8GetDatum(b[1].z)), DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[2].x), Float8GetDatum(b[2].y), Float8GetDatum(b[2].z)));
        
        if (pocvrststr > 0){
          if (fabs(b[1].z) > fabs(b[2].z)){ // Prvy bod je vyssi ako druhy
            if (jemaxmin) {
              for (i = 1; i <= pocvrststr; i++){
                frac = (i * rozostup + fmod(fabs(b[1].z), rozostup)) / fabs(b[1].z - b[2].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "robim 1. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[1].z, rozostup));
                elog(NOTICE, notice);
              }
            }
            else {
              for (i = 0; i < pocvrststr; i++){
                frac = (i * rozostup + fmod(fabs(b[1].z), rozostup)) / fabs(b[1].z - b[2].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "robim 2. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[1].z, rozostup));
                elog(NOTICE, notice);
              }
            }
          }
          else { // Prvy bod je nizsi ako druhy
            if (jemaxmin){
              for (i = 1; i <= pocvrststr; i++){
                //frac = (i * rozostup + rozostup - fmod(b[1].z, rozostup)) / fabs(b[1].z - b[2].z);
                frac = (i * rozostup + fmod(fabs(b[2].z), rozostup)) / fabs(b[2].z - b[1].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "frac je %lf", frac);
                elog(NOTICE, notice);
                sprintf(notice, "robim 3. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[1].z, rozostup));
                elog(NOTICE, notice);
              }
            }
            else {
              for (i = 0; i < pocvrststr; i++){
                //frac = (i * rozostup + rozostup - fmod(b[1].z, rozostup)) / fabs(b[1].z - b[2].z);
                frac = (i * rozostup + fmod(fabs(b[2].z), rozostup)) / fabs(b[2].z - b[1].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "frac je %lf", frac);
                elog(NOTICE, notice);
                sprintf(notice, "robim 4. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[1].z, rozostup));
                elog(NOTICE, notice);
              }
            }  
          }
        }
      }
      sprintf(notice, "aktual je %d", aktual);
      elog(NOTICE, notice);
      
      //Vypocty pre liniu tvorenu tretim a prvym bodom trojuholnika  
      if (special == 3){
        polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
        polebodov[aktual][0] = DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[2].x), Float8GetDatum(b[2].y), Float8GetDatum(b[2].z));
        aktual++;
        polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
        polebodov[aktual][0] = DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[0].x), Float8GetDatum(b[0].y), Float8GetDatum(b[0].z));
        aktual++;  
        sprintf(notice, "Vrstevnica prechadza cez stranu 3,1");
        elog(NOTICE, notice);
      }
      else {
        pocvrststr = abs((int32)(b[2].z / rozostup) - (int32)(b[0].z / rozostup)); 
        
        sprintf(notice, "pocvrststr pred testami je %d", pocvrststr);
        elog(NOTICE, notice); 
        jemaxmin = false;
        if ((fabs(fmod(b[2].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[2].z, rozostup)) - rozostup) < FLT_EPSILON) && (b[2].z == gmax)){
          pocvrststr--;
          jemaxmin = true;
        }
        if ((fabs(fmod(b[0].z, rozostup)) < FLT_EPSILON || fabs(fabs(fmod(b[0].z, rozostup)) - rozostup) < FLT_EPSILON) && (b[0].z == gmax)){
          pocvrststr--;
          jemaxmin = true;
        }
        
        sprintf(notice, "pocvrststr po testoch je %d", pocvrststr);
        elog(NOTICE, notice);
        
        if (fabs(b[2].z) < fabs(b[0].z))
          linia = DirectFunctionCall2(ST_MakeLine, DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[0].x), Float8GetDatum(b[0].y), Float8GetDatum(b[0].z)), DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[2].x), Float8GetDatum(b[2].y), Float8GetDatum(b[2].z)));
        else
          linia = DirectFunctionCall2(ST_MakeLine, DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[2].x), Float8GetDatum(b[2].y), Float8GetDatum(b[2].z)), DirectFunctionCall3(ST_MakePoint, Float8GetDatum(b[0].x), Float8GetDatum(b[0].y), Float8GetDatum(b[0].z)));
        
        if (pocvrststr > 0){
          if (fabs(b[2].z) > fabs(b[0].z)){ // Prvy bod je vyssi ako druhy
            if (jemaxmin){
              for (i = 1; i <= pocvrststr; i++){
                frac = (i * rozostup + fmod(fabs(b[2].z), rozostup)) / fabs(b[2].z - b[0].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "robim 1. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[2].z, rozostup));
                elog(NOTICE, notice);
              }
            }
            else {
              for (i = 0; i < pocvrststr; i++){
                frac = (i * rozostup + fmod(fabs(b[2].z), rozostup)) / fabs(b[2].z - b[0].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "robim 2. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[2].z, rozostup));
                elog(NOTICE, notice);
              }
            }
          }
          else { // Prvy bod je nizsi ako druhy
            if (jemaxmin){
              for (i = 1; i <= pocvrststr; i++){
                //frac = (i * rozostup + rozostup - fmod(b[2].z, rozostup)) / fabs(b[2].z - b[0].z);
                frac = (i * rozostup + fmod(fabs(b[0].z), rozostup)) / fabs(b[0].z - b[2].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "frac je %lf", frac);
                elog(NOTICE, notice);
                sprintf(notice, "robim 3. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[2].z, rozostup));
                elog(NOTICE, notice);
              }
            }
            else {
              for (i = 0; i < pocvrststr; i++){
                //frac = (i * rozostup + rozostup - fmod(b[2].z, rozostup)) / fabs(b[2].z - b[0].z);
                frac = (i * rozostup + fmod(fabs(b[0].z), rozostup)) / fabs(b[0].z - b[2].z);
                polebodov[aktual] = (Datum *) palloc (sizeof(Datum));
                polebodov[aktual][0] = DirectFunctionCall2(ST_Line_Interpolate_Point, linia, Float8GetDatum(frac));
                aktual++;
                sprintf(notice, "frac je %lf", frac);
                elog(NOTICE, notice);
                sprintf(notice, "robim 4. moznost. frac je %lf, i je %d, fmod vychadza %lf", frac, i, fmod(b[2].z, rozostup));
                elog(NOTICE, notice);
              }
            } 
          }
        }
      }
      sprintf(notice, "aktual je %d", aktual);
      elog(NOTICE, notice);
      
      for (i = 0; i < aktual - 1; i++){
        for (j = i + 1; j < aktual; j++){
          if (DatumGetFloat8(DirectFunctionCall1(ST_Z, polebodov[j][0])) < DatumGetFloat8(DirectFunctionCall1(ST_Z, polebodov[i][0]))){
            pomoc = polebodov[i][0];
            polebodov[i][0] = polebodov[j][0];
            polebodov[j][0] = pomoc;
          }
        }
      }
    }
  }
  
  //pocetvrst = 0; // docasne kvoli testovaniu
  if (aktual <= 0){
    PG_RETURN_NULL();
  }
  else {
    vrstevnice = (Datum *) palloc (sizeof(Datum) * aktual / 2);
    for (i = 0; i < aktual / 2; i++){
      vrstevnice[i] = DirectFunctionCall2(ST_MakeLine, polebodov[i*2][0], polebodov[i*2+1][0]);
    }
    o_eltype = get_fn_expr_argtype(fcinfo->flinfo, 0);  
    get_typlenbyvalalign(o_eltype, &o_typlen, &o_typbyval, &o_typalign);
    vysledok = construct_array(vrstevnice, aktual / 2, o_eltype, o_typlen, o_typbyval, o_typalign);
    PG_RETURN_ARRAYTYPE_P(vysledok);
  }     
}

 Datum TIN_PartialDerivativesOnVertexAnglesWeight(PG_FUNCTION_ARGS)
{
  ArrayType *susedia, *vysledok;
  Oid trojeltype, o_eltype;
  int16 trojtyplen, o_typlen;
  bool trojtypbyval, o_typbyval;
  char trojtypalign, o_typalign;
  bool *trojnulls;
  Datum *sustroj, kbod, *elementy;
  int poctroj;
  float8 zx, zy, a, b, c; 
  
  kbod = PG_GETARG_DATUM(0);
  susedia = PG_GETARG_ARRAYTYPE_P(1);
  
  trojeltype = ARR_ELEMTYPE(susedia);
  get_typlenbyvalalign(trojeltype, &trojtyplen, &trojtypbyval, &trojtypalign);
  deconstruct_array(susedia, trojeltype, trojtyplen, trojtypbyval, trojtypalign, &sustroj, &trojnulls, &poctroj);
  
  abcroviny(kbod, sustroj, poctroj, 2, &a, &b, &c);
  
  zx = - a / c;
  zy = - b / c;
  
  elementy = (Datum *) palloc(2 * sizeof(Datum));
  elementy[0] = Float8GetDatum(zx);
  elementy[1] = Float8GetDatum(zy);
   
  o_eltype = FLOAT8OID;  
  get_typlenbyvalalign(o_eltype, &o_typlen, &o_typbyval, &o_typalign);
  
  vysledok = construct_array(elementy, 2, o_eltype, o_typlen, o_typbyval, o_typalign);
  
  PG_RETURN_ARRAYTYPE_P(vysledok);
}

 Datum TIN_Cubature(PG_FUNCTION_ARGS)
{
  float8 z1, z2, z3, zmin, zmax, zmedzi, rozloha, vyska, basez, objemhranola, objem4s, celyobjem;
  PGFunction ST_Z, ST_PointN, ST_Boundary, ST_AsText, ST_Area;
  char filename[] = "postgis-2.2";
  char funcname1[] = "LWGEOM_z_point";
  char funcname2[] = "LWGEOM_pointn_linestring";
  char funcname3[] = "boundary";
  char funcname4[] = "LWGEOM_asText";
  char funcname5[] = "area";
  char *testwkt, notice[500];
  int  slen, minmax[2];
  bool isinverse;
  Datum vstup, medzivystup;
  
  ST_Z = load_external_function(filename, funcname1, 1, NULL);
  ST_PointN = load_external_function(filename, funcname2, 1, NULL);
  ST_Boundary = load_external_function(filename, funcname3, 1, NULL);
  ST_AsText = load_external_function(filename, funcname4, 1, NULL);
  ST_Area = load_external_function(filename, funcname5, 1, NULL);
  
  vstup = PG_GETARG_DATUM(0);
  basez = PG_GETARG_FLOAT8(1);
  isinverse = PG_GETARG_BOOL(2);
  
  medzivystup = DirectFunctionCall1(ST_AsText, vstup);
  slen = VARSIZE(medzivystup) - VARHDRSZ + 1;
  testwkt = (char *) palloc (sizeof(char) * slen);
  memcpy(testwkt, VARDATA(medzivystup), slen);
  testwkt[slen - 1] = '\0'; 
  if (strstr(testwkt, "TRIANGLE Z") == NULL)
    elog(ERROR, "Incorrect input geometry type");
  
  medzivystup = DirectFunctionCall1(ST_Area, vstup);
  rozloha = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(1)));
  z1 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(2)));
  z2 = DatumGetFloat8(medzivystup);
  medzivystup = DirectFunctionCall1(ST_Z, DirectFunctionCall2(ST_PointN, DirectFunctionCall1(ST_Boundary, vstup), Int32GetDatum(3)));
  z3 = DatumGetFloat8(medzivystup);
  
  zmin = z1;
  minmax[0] = 1;
  if (z2 < zmin){
    zmin = z2;
    minmax[0] = 2;
  }
  if (z3 < zmin){
    zmin = z3;
    minmax[0] = 3;
  }
  zmax = z3;
  minmax[1] = 3;
  if (z2 > zmax){
    zmax = z2;
    minmax[1] = 2;
  }
  if (z1 > zmax){
    zmax = z1;
    minmax[1] = 1;
  }
  
  if ((minmax[0] == 1 && minmax[1] == 2) || (minmax[0] == 2 && minmax[1] == 1))
    zmedzi = z3;
  else if ((minmax[0] == 1 && minmax[1] == 3) || (minmax[0] == 3 && minmax[1] == 1))
    zmedzi = z2;
  else if ((minmax[0] == 2 && minmax[1] == 3) || (minmax[0] == 3 && minmax[1] == 2))
    zmedzi = z1;
  
  sprintf(notice, "zmin = %f, zmax = %f, zmedzi = %f, minmax[0] = %d, minmax[1] = %d", zmin, zmax, zmedzi, minmax[0], minmax[1]);
  elog(NOTICE, notice);
  
  vyska = fabs(zmax - zmin);
  if (vyska == 0){
    objem4s = 0;
  }
  else {
    objem4s = (rozloha * vyska * (1 + fabs(zmedzi - zmin) / vyska)) / 3;
  }
  
  sprintf(notice, "objem stvorstena = %f, vyska stvorstena = %f", objem4s, vyska);
  elog(NOTICE, notice);
  
  if (isinverse){
    objem4s = rozloha * vyska - objem4s;
    vyska = fabs(basez - zmax);
    sprintf(notice, "isinverse objem stvorstena = %f, vyska stvorstena = %f", objem4s, vyska);
    elog(NOTICE, notice);
  }
  else {
    vyska = fabs(zmin - basez);
  }
  
  objemhranola = rozloha * vyska;
  celyobjem = objemhranola + objem4s;  
  
  sprintf(notice, "rozloha podstavy = %f, vyska hranola = %f, objemhranola = %f", rozloha, vyska, objemhranola);
  elog(NOTICE, notice);
  
  PG_RETURN_FLOAT8(celyobjem);  
}
