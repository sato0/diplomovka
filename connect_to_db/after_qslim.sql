-- 1.nacitat subor obj
-- 2.rozdelit v a f do dvoch tabuliek
-- 3. vytvorit strukturu po DT ako ma kali

CREATE OR REPLACE FUNCTION after_qslim (input_file text) RETURNS text AS 
$$
BEGIN  
RAISE NOTICE 'vraj sme zacali'; 
SET enable_seqscan = OFF;

EXECUTE 'CREATE TABLE obj_all
(
  id serial NOT NULL,
  category text,
  x numeric(10,2),
  y numeric(10,2),
  z numeric(10,2)
)';
RAISE NOTICE 'vytvorena tabulka obj_all';                                       
  EXECUTE 'COPY obj_all(category,x,y,z) FROM ' || quote_literal(input_file) ||' DELIMITERS' || quote_literal(E' ');
RAISE NOTICE 'udaje nakopirovane';

EXECUTE 'CREATE TABLE points
(
  id serial NOT NULL,
  v text,
  x numeric(10,2),
  y numeric(10,2),
  z numeric(10,2)
)';

EXECUTE 'CREATE TABLE triangles
(
  id serial NOT NULL,
  f text,
  p1id integer,
  p2id integer,
  p3id integer
)';

INSERT INTO points (id,v,x,y,z) SELECT * FROM obj_all WHERE category='v';
INSERT INTO triangles (f,p1id,p2id,p3id) SELECT category,x,y,z FROM obj_all WHERE category='f';

EXECUTE 'ALTER TABLE points ADD COLUMN geom geometry';
UPDATE points
  SET geom = ST_MakePoint(points.x,points.y,points.z);

-- EXECUTE 'ALTER TABLE triangles ADD COLUMN p1 geometry';
-- EXECUTE 'ALTER TABLE triangles ADD COLUMN p2 geometry';
-- EXECUTE 'ALTER TABLE triangles ADD COLUMN p3 geometry';
-- EXECUTE 'ALTER TABLE triangles ADD COLUMN face text';


  RETURN 'koniec';
  END
$$
LANGUAGE plpgsql;

-- Execute examples

DROP TABLE IF EXISTS obj_all;
DROP TABLE IF EXISTS points;
DROP TABLE IF EXISTS triangles;
DROP TABLE IF EXISTS tintable;

SELECT after_qslim('/home/sato/Desktop/dp/connect_to_db/tin.obj');
SELECT tin_createfrompointsdt ('tintable', 'points', 'geom');
-- SELECT tin_topologypids('tintable', 'mariana', 'mariana', 'dp','5432', 'localhost');
-- SELECT tin_topologyntids('tintable');
-- SELECT * FROM obj_all;
-- SELECT * FROM points;
-- SELECT * FROM triangles;
-- SELECT tid,pid1,pid2,pid3 FROM tintable;
-- ALTER TABLE tintable ADD COLUMN wkt text;
-- UPDATE tintable
--   SET wkt = ST_AsText(tintable.geom);
-- SELECT tid,pid1,pid2,pid3,wkt FROM tintable;
-- -- SELECT _tin_topology(1, 6, 'tintable', 1);
-- SELECT tin_topologyntids('tintable');
SELECT tid,pid1,pid2,pid3,ntid1,ntid2,ntid3 FROM tintable;

-- SELECT (g.gdump).path, ST_AsText((g.gdump).geom) as wkt
--   FROM
--     (SELECT 
--        ST_DumpPoints( (select tintable.geom from tintable limit 1) ) AS gdump
--     ) AS g;

 -- SELECT tin_flip(5, 1, 'tintable');
-- ALTER TABLE tintable ADD COLUMN area double precision;
-- UPDATE tintable
--   SET area = TIN_Area3D(tintable.geom);
-- SELECT tid,pid1,pid2,pid3,area FROM tintable;
 
WITH sekvencia AS (
  SELECT pid1 AS pid FROM tintable UNION
  SELECT pid2 FROM tintable UNION
  SELECT pid3 FROM tintable
)
SELECT pid, ST_X(TIN_GetPoint(pid, 'tintable')) AS X,
  ST_Y(TIN_GetPoint(pid, 'tintable')) AS Y,
  (TIN_PartialDerivativesOnVertex(pid,'tintable'))[1] AS zx,
  (TIN_PartialDerivativesOnVertex(pid,'tintable'))[2] AS zy FROM sekvencia ORDER BY pid;

-- poslat feciskaninovi, ked bude hotove