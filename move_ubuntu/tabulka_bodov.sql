-- /home/sato/Desktop/dp/connect_to_db/sample_house26

-- CREATE TABLE coordinates
-- (
--   id serial NOT NULL,
--   x numeric(10,2),
--   y numeric(10,2),
--   z numeric(10,2),
--   the_geom geometry,
--   CONSTRAINT coordinates_pkey PRIMARY KEY (id)
-- 
-- );

--copy coordinates(x,y,z) FROM '/home/sato/Desktop/dp/connect_to_db/sample_house26' DELIMITERS ' ' ;
--select id,x,y,z from coordinates order by id desc limit 10 ;
-- UPDATE coordinates
-- SET the_geom = ST_MakePoint(coordinates.x,coordinates.y,coordinates.z);
--  ALTER TABLE coordinates
-- 	ADD point_wkt text;
-- UPDATE coordinates
-- SET point_wkt = ST_AsText(ST_MakePoint(coordinates.x,coordinates.y,coordinates.z));

--select * from coordinates limit 10;
-- -- 
-- SELECT 
-- 	ST_AsText((ST_Dump(geom)).geom) as polygon_wkt,
-- 	(ST_Dump(geom)).geom as polygon_geom,
-- 	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),1 )) as point1,
-- 	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),2)) as point2,
-- 	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom),3)) as point3,
-- 	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom),4)) as point14
-- 	into table polygons 
-- FROM ( SELECT ST_DelaunayTriangles(ST_Collect(the_geom)) As geom
-- FROM coordinates)as foo ;

--select * from polygons limit 10;

--vlozenie udajov a vyber cca 100 s/ 65000 trojuhonikov  

-- CREATE INDEX index_point_wkt
-- ON coordinates (point_wkt);
-- CREATE INDEX index_point1
-- ON polygons (point1);
-- CREATE INDEX index_point2
-- ON polygons (point2);
-- CREATE INDEX index_point3
-- ON polygons (point3);

-- ALTER TABLE polygons
-- 	ADD COLUMN pid1 integer;
-- ALTER TABLE polygons
-- 	ADD COLUMN pid2 integer;
-- ALTER TABLE polygons
-- 	ADD COLUMN pid3 integer;
-- ALTER TABLE polygons
-- 	ADD COLUMN face text;
-- ALTER TABLE coordinates
-- 	ADD COLUMN vertex text;
--65174 polygonov



-- UPDATE polygons SET pid1 = coordinates.id FROM coordinates
--   WHERE point_wkt = polygons.point1;
-- UPDATE polygons SET pid2 = coordinates.id FROM coordinates
--   WHERE point_wkt = polygons.point2;
-- UPDATE polygons SET pid3 = coordinates.id FROM coordinates
--   WHERE point_wkt = polygons.point3;
--UPDATE polygons SET face = 'f';
--UPDATE coordinates SET vertex = 'v';
--######################################################## UROBENE
--select * from coordinates order by id asc limit 10;

--SELECT COUNT(*) FROM polygons;

-- COPY (SELECT vertex,x,y,z FROM coordinates ORDER BY id ASC) TO '/home/sato/Desktop/dp/connect_to_db/vertex_from_pg.obj' DELIMITER ' ';
-- COPY (SELECT face,pid1,pid2,pid3 FROM polygons ) TO '/home/sato/Desktop/dp/connect_to_db/face_from_pg.obj' DELIMITER ' ';

--v bash cat /home/sato/Desktop/dp/connect_to_db/vertex_from_pg.obj /home/sato/Desktop/dp/connect_to_db/face_from_pg.obj  > /home/sato/Desktop/dp/connect_to_db/tin_from_pg.obj