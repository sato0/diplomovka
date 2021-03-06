﻿--/home/sato/Desktop/dp/connect_to_db/

CREATE OR REPLACE FUNCTION tin_topology (input_file text, output_dir text) RETURNS text AS 
$$
BEGIN  

SET enable_seqscan = OFF;

EXECUTE 'CREATE TABLE coordinates_tmp
(
  id serial NOT NULL,
  x numeric(10,2),
  y numeric(10,2),
  z numeric(10,2),
  the_geom geometry,
  CONSTRAINT coordinates_tmp_pkey PRIMARY KEY (id)

)';
RAISE NOTICE 'vytvorena tabulka coordinates_tmp';                                       
  EXECUTE 'COPY coordinates_tmp(x,y,z) FROM ' || quote_literal(input_file) ||' DELIMITERS' || quote_literal(E' ');
RAISE NOTICE 'udaje nakopirovane';

UPDATE coordinates_tmp
	SET the_geom = ST_MakePoint(coordinates_tmp.x,coordinates_tmp.y,coordinates_tmp.z);
ALTER TABLE coordinates_tmp
	ADD point_wkt text;
UPDATE coordinates_tmp
SET point_wkt = ST_AsText(ST_MakePoint(coordinates_tmp.x,coordinates_tmp.y,coordinates_tmp.z));

explain CREATE TABLE polygons_tmp AS 
SELECT 
	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),1 )) as point1,
	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom ),2)) as point2,
	ST_AsText(ST_PointN(ST_ExteriorRing((ST_Dump(geom)).geom),3)) as point3  
	FROM ( SELECT ST_DelaunayTriangles(ST_Collect(the_geom)) As geom
	FROM coordinates_tmp)as foo ;

RAISE NOTICE 'tin vytvorena';

EXECUTE 'CREATE INDEX ON coordinates_tmp (point_wkt)';
EXECUTE 'CREATE INDEX ON polygons_tmp (point1)';
EXECUTE 'CREATE INDEX ON polygons_tmp (point2)';
EXECUTE 'CREATE INDEX ON polygons_tmp (point3)';
EXECUTE 'CREATE INDEX ON coordinates_tmp (x)';
EXECUTE 'CREATE INDEX ON coordinates_tmp (y)';
EXECUTE 'CREATE INDEX ON coordinates_tmp (z)';

RAISE NOTICE 'indexy vytvorene';

EXECUTE 'ALTER TABLE polygons_tmp ADD COLUMN pid1 integer';
EXECUTE 'ALTER TABLE polygons_tmp ADD COLUMN pid2 integer';
EXECUTE 'ALTER TABLE polygons_tmp ADD COLUMN pid3 integer';
EXECUTE 'ALTER TABLE polygons_tmp ADD COLUMN face text';
EXECUTE 'ALTER TABLE coordinates_tmp ADD COLUMN vertex text';


EXECUTE 'explain UPDATE polygons_tmp SET pid1 = coordinates_tmp.id FROM coordinates_tmp
  WHERE point_wkt = polygons_tmp.point1';
EXECUTE 'UPDATE polygons_tmp SET pid2 = coordinates_tmp.id FROM coordinates_tmp
  WHERE point_wkt = polygons_tmp.point2';
EXECUTE 'UPDATE polygons_tmp SET pid3 = coordinates_tmp.id FROM coordinates_tmp
  WHERE point_wkt = polygons_tmp.point3';

  
UPDATE polygons_tmp SET face = 'f';
UPDATE coordinates_tmp SET vertex = 'v';

RAISE NOTICE 'topologia vytvorena';

EXECUTE 'COPY (SELECT vertex,x,y,z FROM coordinates_tmp ORDER BY id ASC) TO ' || quote_literal(output_dir || '/vertex.obj') ||' DELIMITER '|| quote_literal(E' ') ;
EXECUTE 'COPY (SELECT face,pid1,pid2,pid3 FROM polygons_tmp ) TO ' || quote_literal(output_dir ||'/face.obj') ||' DELIMITER'|| quote_literal(E' ') ;

RAISE NOTICE 'subory vytvorene';

DROP TABLE IF EXISTS coordinates_tmp;
DROP TABLE IF EXISTS polygons_tmp;

SET enable_seqscan = ON;
	
  RETURN 'STRUKTURA VYTVORENA V SUBOROCH';
  END
$$
LANGUAGE plpgsql;

