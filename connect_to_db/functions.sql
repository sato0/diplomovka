
CREATE OR REPLACE FUNCTION triangulation_for_qslim(input_file text, output_dir text, output_file text) RETURNS text AS 
$$
BEGIN 

  DROP TABLE IF EXISTS temp_points;
  DROP TABLE IF EXISTS triangles;

  EXECUTE 'CREATE TABLE temp_points
  (
    id serial NOT NULL,
    x double precision,
    y double precision,
    z double precision,
    geom geometry,
    point_wkt text
  )';

  EXECUTE 'COPY temp_points(x,y,z) FROM ' || quote_literal(input_file) ||' DELIMITERS' || quote_literal(E' ');
  RAISE NOTICE 'DATA LOADED';
  UPDATE temp_points
    SET geom = ST_MakePoint(temp_points.x,temp_points.y,temp_points.z);
  UPDATE temp_points
    SET point_wkt = ST_AsText(geom);

  CREATE TABLE triangles AS 
    SELECT 
      ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),1 )) as point1,
      ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),2)) as point2,
      ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom),3)) as point3
      FROM ( SELECT ST_DelaunayTriangles(ST_Collect(geom)) As geom
      FROM temp_points)as tp ;
  RAISE NOTICE 'TRIANGULATION EXECUTED';
  EXECUTE 'CREATE INDEX ON temp_points (id)';
  EXECUTE 'CREATE INDEX ON temp_points (geom)';
  EXECUTE 'CREATE INDEX ON temp_points (point_wkt)';
  EXECUTE 'CREATE INDEX ON temp_points (x)';
  EXECUTE 'CREATE INDEX ON temp_points (y)';
  EXECUTE 'CREATE INDEX ON temp_points (z)';

  EXECUTE 'CREATE INDEX ON triangles (point1)';
  EXECUTE 'CREATE INDEX ON triangles (point2)';
  EXECUTE 'CREATE INDEX ON triangles (point3)';

  EXECUTE 'ALTER TABLE triangles ADD COLUMN pid1 integer';
  EXECUTE 'ALTER TABLE triangles ADD COLUMN pid2 integer';
  EXECUTE 'ALTER TABLE triangles ADD COLUMN pid3 integer';
  EXECUTE 'ALTER TABLE triangles ADD COLUMN face text';
  EXECUTE 'ALTER TABLE temp_points ADD COLUMN vertex text';

  EXECUTE 'UPDATE triangles SET pid1 = temp_points.id FROM temp_points
  WHERE point_wkt = triangles.point1';
  EXECUTE 'UPDATE triangles SET pid2 = temp_points.id FROM temp_points
  WHERE point_wkt = triangles.point2';
  EXECUTE 'UPDATE triangles SET pid3 = temp_points.id FROM temp_points
  WHERE point_wkt = triangles.point3';
  RAISE NOTICE 'POINTS IDS SELECTED';
  UPDATE triangles SET face = 'f';
  UPDATE temp_points SET vertex = 'v';

  ALTER TABLE triangles RENAME COLUMN face TO vertex;
  ALTER TABLE triangles RENAME COLUMN pid1 TO x;
  ALTER TABLE triangles RENAME COLUMN pid2 TO y;
  ALTER TABLE triangles RENAME COLUMN pid3 TO z;

  EXECUTE 'COPY ((SELECT vertex,x,y,z FROM temp_points ORDER BY id ASC) UNION ALL (SELECT vertex,x,y,z FROM triangles) )TO ' || quote_literal(output_dir || '/' || output_file || '.obj') ||' DELIMITER '|| quote_literal(E' ') ;

  RETURN 'TIN CREATED IN FILE' || quote_literal(output_file) ||'.obj';
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION TIN_for_pg3angles(input_obj_file text, srid text) RETURNS text AS 
$$
BEGIN 

  DROP TABLE IF EXISTS temp_obj;
  DROP TABLE IF EXISTS temp_points;
  DROP TABLE IF EXISTS tin;

  EXECUTE 'CREATE TABLE temp_obj
  (
    id serial NOT NULL,
    category text,
    x double precision,
    y double precision,
    z double precision
  )';

  EXECUTE 'COPY temp_obj(category,x,y,z) FROM ' || quote_literal(input_obj_file) || 'DELIMITER ' || quote_literal(E' ');

  EXECUTE 'CREATE TABLE temp_points
  (
    id serial NOT NULL,
    v text,
    x double precision,
    y double precision,
    z double precision
  )';

  EXECUTE 'CREATE TABLE tin
  (
    tid serial NOT NULL,
    f text,
    p1id integer,
    p2id integer,
    p3id integer,
    wkt text,
    geom geometry,
    p1x double precision,
    p1y double precision,
    p1z double precision,
    p2x double precision,
    p2y double precision,
    p2z double precision,
    p3x double precision,
    p3y double precision,
    p3z double precision
  )';

  INSERT INTO temp_points (id,v,x,y,z) SELECT * FROM temp_obj WHERE category='v';
  INSERT INTO tin (f,p1id,p2id,p3id) SELECT category,x,y,z FROM temp_obj WHERE category='f';

  EXECUTE 'UPDATE tin SET p1x = temp_points.x FROM temp_points WHERE temp_points.id = tin.p1id';
  EXECUTE 'UPDATE tin SET p2x = temp_points.x FROM temp_points WHERE temp_points.id = tin.p2id';
  EXECUTE 'UPDATE tin SET p3x = temp_points.x FROM temp_points WHERE temp_points.id = tin.p3id';
  EXECUTE 'UPDATE tin SET p1y = temp_points.y FROM temp_points WHERE temp_points.id = tin.p1id';
  EXECUTE 'UPDATE tin SET p2y = temp_points.y FROM temp_points WHERE temp_points.id = tin.p2id';
  EXECUTE 'UPDATE tin SET p3y = temp_points.y FROM temp_points WHERE temp_points.id = tin.p3id';
  EXECUTE 'UPDATE tin SET p1z = temp_points.z FROM temp_points WHERE temp_points.id = tin.p1id';
  EXECUTE 'UPDATE tin SET p2z = temp_points.z FROM temp_points WHERE temp_points.id = tin.p2id';
  EXECUTE 'UPDATE tin SET p3z = temp_points.z FROM temp_points WHERE temp_points.id = tin.p3id';

  UPDATE tin
    SET wkt = 'SRID=' || srid || ';TRIANGLE((' || p1x || ' ' || p1y || ' ' || p1z || ',' || p2x || ' ' || p2y || ' '|| p2z || 
              ',' || p3x || ' ' || p3y || ' '|| p3z || ',' || p1x || ' ' || p1y || ' '|| p1z || '))';

  UPDATE tin
    SET geom = ST_GeomFromEWKT(wkt);

  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p1x';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p1y';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p1z';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p2x';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p2y';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p2z';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p3x';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p3y';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p3z';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS wkt';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p1id';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p2id';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS p3id';
  EXECUTE 'ALTER TABLE tin DROP COLUMN IF EXISTS f';


  RETURN 'CREATED TABLE tin';

END
$$
LANGUAGE plpgsql;


