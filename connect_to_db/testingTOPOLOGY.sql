  DROP TABLE IF EXISTS begin_points;
  DROP TABLE IF EXISTS begin_TIN;
  DROP TABLE IF EXISTS implicit_points;

  CREATE TABLE begin_points
  (
    id serial NOT NULL,
    x numeric(10,2),
    y numeric(10,2),
    z numeric(10,2),
    geom geometry,
    point_wkt text
  );

  COPY begin_points(x,y,z) FROM '/home/sato/Desktop/dp/las_data_samples/clip_txt/ctsmall2495.txt' DELIMITERS ' ';
  UPDATE begin_points
    SET geom = ST_MakePoint(begin_points.x,begin_points.y,begin_points.z);
  UPDATE begin_points
    SET point_wkt = ST_AsText(geom);

  CREATE TABLE begin_TIN AS 
    SELECT 
      ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),1 )) as point1,
      ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),2)) as point2,
      ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom),3)) as point3  
      FROM ( SELECT ST_DelaunayTriangles(ST_Collect(geom)) As geom
      FROM begin_points)as tp ;

  CREATE INDEX ON begin_points (id);
  CREATE INDEX ON begin_points (geom);
  CREATE INDEX ON begin_points (point_wkt);
  CREATE INDEX ON begin_points (x);
  CREATE INDEX ON begin_points (y);
  CREATE INDEX ON begin_points (z);

  CREATE INDEX ON begin_TIN (point1);
  CREATE INDEX ON begin_TIN (point2);
  CREATE INDEX ON begin_TIN (point3);

  ALTER TABLE begin_TIN ADD COLUMN p1 geometry;
  ALTER TABLE begin_TIN ADD COLUMN p2 geometry;
  ALTER TABLE begin_TIN ADD COLUMN p3 geometry;
  ALTER TABLE begin_TIN ADD COLUMN wkt text;
  ALTER TABLE begin_TIN ADD COLUMN triangle geometry;

  UPDATE begin_TIN SET p1 = begin_points.geom FROM begin_points
  WHERE point_wkt = begin_TIN.point1;
  UPDATE begin_TIN SET p2 = begin_points.geom FROM begin_points
  WHERE point_wkt = begin_TIN.point2;
  UPDATE begin_TIN SET p3 = begin_points.geom FROM begin_points
  WHERE point_wkt = begin_TIN.point3;

  UPDATE begin_TIN SET wkt = 'SRID=' ||21781 || ';TRIANGLE((' || ST_X(p1) || ' ' || ST_Y(p1) || ' ' || ST_Z(p1) || ',' || ST_X(p2) || ' ' || ST_Y(p2) || ' '|| ST_Z(p2) || 
              ',' || ST_X(p3) || ' ' || ST_Y(p3) || ' '|| ST_Z(p3) || ',' || ST_X(p1) || ' ' || ST_Y(p1) || ' '|| ST_Z(p1) || '))';

  UPDATE begin_TIN
    SET triangle = ST_GeomFromEWKT(wkt);

    CREATE TABLE implicit_points AS (SELECT p1 FROM begin_TIN) UNION (SELECT p2 FROM begin_TIN) UNION (SELECT p3 FROM begin_TIN);
  
  ALTER TABLE implicit_points RENAME COLUMN p1 TO geom;
  ALTER TABLE implicit_points ADD COLUMN id serial;
  ALTER TABLE begin_TIN ADD COLUMN pid1 integer;
  ALTER TABLE begin_TIN ADD COLUMN pid2 integer;
  ALTER TABLE begin_TIN ADD COLUMN pid3 integer;

  UPDATE begin_TIN SET pid1 = implicit_points.id FROM implicit_points
  WHERE begin_TIN.p1 = implicit_points.geom;
  UPDATE begin_TIN SET pid2 = implicit_points.id FROM implicit_points
  WHERE begin_TIN.p2 = implicit_points.geom;
  UPDATE begin_TIN SET pid3 = implicit_points.id FROM implicit_points
  WHERE begin_TIN.p3 = implicit_points.geom;

  ALTER TABLE implicit_points ADD COLUMN x double precision;
  ALTER TABLE implicit_points ADD COLUMN y double precision;
  ALTER TABLE implicit_points ADD COLUMN z double precision;

  ALTER TABLE implicit_points ADD COLUMN vertex text;
  ALTER TABLE begin_TIN ADD COLUMN face text;
  UPDATE begin_TIN SET face = 'f';
  UPDATE implicit_points SET vertex = 'v';
  UPDATE implicit_points SET x = ST_X(geom);
  UPDATE implicit_points SET y = ST_Y(geom);
  UPDATE implicit_points SET z = ST_Z(geom);

  ALTER TABLE begin_TIN RENAME COLUMN face TO vertex;
  ALTER TABLE begin_TIN RENAME COLUMN pid1 TO x;
  ALTER TABLE begin_TIN RENAME COLUMN pid2 TO y;
  ALTER TABLE begin_TIN RENAME COLUMN pid3 TO z;

  COPY ((SELECT vertex,x,y,z FROM implicit_points ORDER BY id ASC) UNION ALL (SELECT vertex,x,y,z FROM begin_TIN) )TO '/home/sato/Desktop/dp/connect_to_db/TESTTIN.smf' DELIMITER ' ';