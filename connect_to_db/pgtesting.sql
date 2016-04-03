--   drop table  if exists base_points;
--   drop table  if exists crater10000;
--   CREATE TABLE base_points
--   (
--     id serial NOT NULL,
--     x numeric(10,2),
--     y numeric(10,2),
--     z numeric(10,2),
--     geom geometry,
--     point_wkt text
--   );

--   COPY base_points(x,y,z) FROM '/home/sato/Desktop/dp/las_data_samples/clip_txt/ctsmall2495.txt' DELIMITERS ' ';
--   UPDATE base_points
--     SET geom = ST_MakePoint(base_points.x,base_points.y,base_points.z);
--   UPDATE base_points
--     SET point_wkt = ST_AsText(geom);

-- select TIN_CreateFromPointsDT('crater10000', 'base_points', 'geom');


ALTER TABLE crater10000 ADD COLUMN aspect numeric(6,2);
  UPDATE crater10000
    SET aspect = TIN_AspectOfTriangle(geom);

ALTER TABLE crater10000 ADD COLUMN slope numeric(7,2);
  UPDATE crater10000
    SET slope = TIN_SlopeOfTriangle(geom);

ALTER TABLE crater10000 ADD COLUMN centroid geometry;
  UPDATE crater10000
    SET centroid = TIN_Centroid(geom);

ALTER TABLE crater10000 ADD COLUMN height numeric(10,2);
  UPDATE crater10000
    SET height = TIN_LinearZ(geom,centroid);

ALTER TABLE crater10000 ADD COLUMN area numeric(10,2);
  UPDATE crater10000
    SET area = TIN_Area3D(geom);

-- ALTER TABLE crater10000 ADD COLUMN cubature numeric(10,2);
--   UPDATE crater10000
--     SET cubature = TIN_LinearZ(geom,centroid);