--SET enable_seqscan = OFF;
--DROP TABLE IF EXISTS coordinates_tmp;
--DROP TABLE IF EXISTS polygons_tmp;

CREATE TABLE coordinates_tmp
(
  id serial NOT NULL,
  x numeric(10,2),
  y numeric(10,2),
  z numeric(10,2),
  the_geom geometry

);
--437ms

------------------------------------------------
COPY coordinates_tmp(x,y,z) FROM '/home/sato/Desktop/dp/connect_to_db/house2txt' DELIMITERS ' ';
--12683 ms

------------------------------------------------
UPDATE coordinates_tmp
	SET the_geom = ST_MakePoint(coordinates_tmp.x,coordinates_tmp.y,coordinates_tmp.z);
ALTER TABLE coordinates_tmp
	ADD point_wkt text;
UPDATE coordinates_tmp
SET point_wkt = ST_AsText(ST_MakePoint(coordinates_tmp.x,coordinates_tmp.y,coordinates_tmp.z));
--108443 ms
------------------------------------------------
-- 
CREATE TABLE polygons_tmp AS 
SELECT  
	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),1 )) as point1,
	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),2)) as point2,
	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom),3)) as point3 
	FROM ( SELECT ST_DelaunayTriangles(ST_Collect(the_geom),0,0) As geom
	FROM coordinates_tmp)as foo ;
-- --
-- 

------------------------------------------------
 CREATE INDEX ON coordinates_tmp (point_wkt);
 CREATE INDEX ON polygons_tmp (point1);
 CREATE INDEX ON polygons_tmp (point2);
 CREATE INDEX ON polygons_tmp (point3);
 CREATE INDEX ON coordinates_tmp (x);
 CREATE INDEX ON coordinates_tmp (y);
 CREATE INDEX ON coordinates_tmp (z);
--
------------------------------------------------


 ALTER TABLE polygons_tmp ADD COLUMN pid1 integer;
 ALTER TABLE polygons_tmp ADD COLUMN pid2 integer;
 ALTER TABLE polygons_tmp ADD COLUMN pid3 integer;
 ALTER TABLE polygons_tmp ADD COLUMN face text;
 ALTER TABLE coordinates_tmp ADD COLUMN vertex text;

--tuto cast treba optimalizovat

 UPDATE polygons_tmp SET pid1 = coordinates_tmp.id FROM coordinates_tmp
  WHERE point_wkt = polygons_tmp.point1;
 UPDATE polygons_tmp SET pid2 = coordinates_tmp.id FROM coordinates_tmp
  WHERE point_wkt = polygons_tmp.point2;
 UPDATE polygons_tmp SET pid3 = coordinates_tmp.id FROM coordinates_tmp
  WHERE point_wkt = polygons_tmp.point3;

--tuto cast treba optimalizovat

  
UPDATE polygons_tmp SET face = 'f';
UPDATE coordinates_tmp SET vertex = 'v';


 COPY (SELECT vertex,x,y,z FROM coordinates_tmp ORDER BY id ASC) TO '/home/sato/Desktop/dp/connect_to_db/vertex.obj'  DELIMITER ' ';
 COPY (SELECT face,pid1,pid2,pid3 FROM polygons_tmp ) TO '/home/sato/Desktop/dp/connect_to_db/face.obj' DELIMITER ' ' ;

DROP TABLE IF EXISTS coordinates_tmp;
DROP TABLE IF EXISTS polygons_tmp;

SET enable_seqscan = ON;
