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
--
-- Subor: pg3angles--1.01.sql  Autor: Mgr. Martin Kalivoda  Datum: 27.5.2015
--
-- Tento subor implementuje funkcie extenzie (extension) pg3angles do databazoveho
-- systemu PostgreSQL a jeho priestorovej nadstavby PostGIS. Dotknute funkcie
-- rozsiruju moznosti prace s nepravidelnymi trojuholnikovymi sietami (TIN).
--
-- Zrusene funkcie od verzie 1.01:
--    TIN_PrepareForPg3angles
--    TIN_GrantTinBehaviour
--    TIN_EditNeighboursInfo
--    TIN_ExtendM
--    TIN_GetTid
--    TIN_GetNeighboursTids
--

-- Pomocou prikazu \echo zabranime nacitaniu prikazov tohto scriptu samostatne (mimo extenzie)
\echo Use "CREATE EXTENSION pg3angles" to load this file. \quit 

CREATE OR REPLACE FUNCTION tin_createfrompointsdt (tintable text, pntstable text, pntsgeomcol text) RETURNS text AS -- Upravena od verzie 1.01
$$
BEGIN                                         
  EXECUTE 'CREATE TABLE ' || tintable || ' (tid serial, geom geometry)';
  EXECUTE 'INSERT INTO ' || tintable || ' (geom)
    SELECT (elementy).geom FROM (SELECT ST_Dump(ST_DelaunayTriangles(ST_Collect(' || pntsgeomcol || '), 0, 2)) AS elementy 
		FROM ' || pntstable || ') AS subselekcia';
  RETURN 'OK';
END
$$
LANGUAGE plpgsql;
  
CREATE OR REPLACE FUNCTION tin_topologypids(tintable text, username text, password text, dbname text, port text = '5432', host text = 'localhost') RETURNS text AS -- Nova od verzie 1.01
$$
DECLARE
  r RECORD;
  i integer;
  j integer;
  connarray text[];
  conn text;
BEGIN
  CREATE EXTENSION IF NOT EXISTS dblink;
  SELECT dblink_get_connections() INTO connarray;
  IF connarray[1] IS NOT NULL THEN
    FOREACH conn IN ARRAY connarray
    LOOP
      EXECUTE 'SELECT dblink_disconnect(' || quote_literal(conn) || ')';
    END LOOP;
  END IF;
  
  DROP TABLE IF EXISTS temppid1;
  DROP TABLE IF EXISTS temppid2;
  DROP TABLE IF EXISTS temppid3;
  
  EXECUTE 'SELECT dblink_connect(''c1'', ''host=' || host || ' port=' || port || ' user=' || username || ' password=' || password || ' dbname=' || dbname || ' '')';
  EXECUTE 'SELECT dblink_exec(''c1'', ''DROP TABLE IF EXISTS temptplg1'')';
  EXECUTE 'SELECT dblink_exec(''c1'', ''DROP TABLE IF EXISTS temptplg2'')';
  EXECUTE 'SELECT dblink_exec(''c1'', ''DROP TABLE IF EXISTS temptplg3'')';
  EXECUTE 'SELECT dblink_exec(''c1'', ''DROP TABLE IF EXISTS temptplg4'')';
  EXECUTE 'SELECT dblink_exec(''c1'', ''DROP TABLE IF EXISTS temptplg5'')';
  EXECUTE 'SELECT dblink_exec(''c1'', ''DROP TABLE IF EXISTS temppnts'')';
  EXECUTE 'SELECT dblink_exec(''c1'', ''CREATE TABLE temptplg1 (tid integer, pid1 integer, pid2 integer, pid3 integer)'')';
  RAISE NOTICE 'temptplg1 vytvorena';
  EXECUTE 'SELECT dblink_exec(''c1'', ''CREATE TABLE temptplg2 (tid integer, pid1 integer, pid2 integer, pid3 integer)'')';
  RAISE NOTICE 'temptplg2 vytvorena';
  EXECUTE 'SELECT dblink_exec(''c1'', ''CREATE TABLE temptplg3 (tid integer, pid1 integer, pid2 integer, pid3 integer)'')';
  RAISE NOTICE 'temptplg3 vytvorena';
  EXECUTE 'SELECT dblink_exec(''c1'', ''CREATE TABLE temptplg4 (tid integer, pid1 integer, pid2 integer, pid3 integer)'')';
  RAISE NOTICE 'temptplg4 vytvorena';
  EXECUTE 'SELECT dblink_exec(''c1'', ''CREATE TABLE temptplg5 (tid integer, pid1 integer, pid2 integer, pid3 integer)'')';
  RAISE NOTICE 'temptplg5 vytvorena';
  EXECUTE 'SELECT dblink_exec(''c1'', ''CREATE TABLE temppnts (id serial, geom geometry)'')';
  EXECUTE 'SELECT dblink_exec(''c1'', ''INSERT INTO temppnts (geom) SELECT DISTINCT ON (ST_AsBinary((subselect.dump).geom)) (subselect.dump).geom AS geom FROM
	                           (SELECT ST_DumpPoints(geom) AS dump FROM ' || tintable || ') AS subselect WHERE (dump).path[2] < 4'')';
  EXECUTE 'SELECT dblink_disconnect(''c1'')';
  RAISE NOTICE 'temppnts vytvorena a naplnena';
  
  EXECUTE 'SELECT Count(*) FROM temppnts' INTO j;
  RAISE NOTICE 'pocet riadkov je %', j;
  EXECUTE 'SELECT dblink_connect(''c1'', ''host=' || host || ' port=' || port || ' user=' || username || ' password=' || password || ' dbname=' || dbname || ' '')';
  EXECUTE 'SELECT dblink_connect(''c2'', ''host=' || host || ' port=' || port || ' user=' || username || ' password=' || password || ' dbname=' || dbname || ' '')';
  EXECUTE 'SELECT dblink_connect(''c3'', ''host=' || host || ' port=' || port || ' user=' || username || ' password=' || password || ' dbname=' || dbname || ' '')';
  EXECUTE 'SELECT dblink_connect(''c4'', ''host=' || host || ' port=' || port || ' user=' || username || ' password=' || password || ' dbname=' || dbname || ' '')';
  EXECUTE 'SELECT dblink_connect(''c5'', ''host=' || host || ' port=' || port || ' user=' || username || ' password=' || password || ' dbname=' || dbname || ' '')';
  EXECUTE 'SELECT dblink_send_query(''c1'', ''SELECT _TIN_Topology(1, ' || (j / 5)::text || ', ''' || quote_literal(tintable) || ''', 1)'')';
  EXECUTE 'SELECT dblink_send_query(''c2'', ''SELECT _TIN_Topology(' || (1 + j / 5)::text || ', ' || (2 * j / 5)::text || ', ''' || quote_literal(tintable) || ''', 2)'')';
  EXECUTE 'SELECT dblink_send_query(''c3'', ''SELECT _TIN_Topology(' || (1 + 2 * j / 5)::text || ', ' || (3 * j / 5)::text || ', ''' || quote_literal(tintable) || ''', 3)'')';
  EXECUTE 'SELECT dblink_send_query(''c4'', ''SELECT _TIN_Topology(' || (1 + 3 * j / 5)::text || ', ' || (4 * j / 5)::text || ', ''' || quote_literal(tintable) || ''', 4)'')';
  EXECUTE 'SELECT dblink_send_query(''c5'', ''SELECT _TIN_Topology(' || (1 + 4 * j / 5)::text || ', ' || j::text || ', ''' || quote_literal(tintable) || ''', 5)'')';
  PERFORM pg_sleep(2);
  WHILE dblink_is_busy('c1') = 1 OR dblink_is_busy('c2') = 1 OR dblink_is_busy('c3') = 1 OR dblink_is_busy('c4') = 1 OR dblink_is_busy('c5') = 1
  LOOP 
    RAISE NOTICE 'LOOP';
    PERFORM dblink_is_busy('c1');
    PERFORM dblink_is_busy('c2');
    PERFORM dblink_is_busy('c3');
    PERFORM dblink_is_busy('c4');
    PERFORM dblink_is_busy('c5');
    PERFORM pg_sleep(1); 
  END LOOP;
  PERFORM dblink_disconnect('c1');
  PERFORM dblink_disconnect('c2');
  PERFORM dblink_disconnect('c3');
  PERFORM dblink_disconnect('c4');
  PERFORM dblink_disconnect('c5');
  DROP TABLE temppnts;
  
  EXECUTE 'ALTER TABLE ' || tintable || ' DROP COLUMN IF EXISTS pid1';
  EXECUTE 'ALTER TABLE ' || tintable || ' DROP COLUMN IF EXISTS pid2';
  EXECUTE 'ALTER TABLE ' || tintable || ' DROP COLUMN IF EXISTS pid3';
  EXECUTE 'ALTER TABLE ' || tintable || ' ADD COLUMN pid1 integer';
  EXECUTE 'ALTER TABLE ' || tintable || ' ADD COLUMN pid2 integer';
  EXECUTE 'ALTER TABLE ' || tintable || ' ADD COLUMN pid3 integer';
  
  EXECUTE 'CREATE INDEX ON ' || tintable || ' (tid)'; 
  
  CREATE TABLE temppid1 (tid integer, pid1 integer);
  WITH tidpid1 AS (
	 SELECT tid, pid1 FROM temptplg1 WHERE pid1 IS NOT NULL UNION
	 SELECT tid, pid1 FROM temptplg2 WHERE pid1 IS NOT NULL UNION
	 SELECT tid, pid1 FROM temptplg3 WHERE pid1 IS NOT NULL UNION
	 SELECT tid, pid1 FROM temptplg4 WHERE pid1 IS NOT NULL UNION
	 SELECT tid, pid1 FROM temptplg5 WHERE pid1 IS NOT NULL ORDER BY tid
  )
  INSERT INTO temppid1 SELECT * FROM tidpid1;
  RAISE NOTICE 'temppid1 naplnena';
  CREATE INDEX ON temppid1 (tid);
  EXECUTE 'UPDATE ' || tintable ||' AS a SET pid1 = (SELECT pid1 FROM temppid1 AS b WHERE a.tid = b.tid)';
  RAISE NOTICE 'pid1 updatnute';
  
  CREATE TABLE temppid2 (tid integer, pid2 integer);
  WITH tidpid2 AS (
	 SELECT tid, pid2 FROM temptplg1 WHERE pid2 IS NOT NULL UNION
	 SELECT tid, pid2 FROM temptplg2 WHERE pid2 IS NOT NULL UNION
	 SELECT tid, pid2 FROM temptplg3 WHERE pid2 IS NOT NULL UNION
	 SELECT tid, pid2 FROM temptplg4 WHERE pid2 IS NOT NULL UNION
	 SELECT tid, pid2 FROM temptplg5 WHERE pid2 IS NOT NULL ORDER BY tid
  )
  INSERT INTO temppid2 SELECT * FROM tidpid2;
  RAISE NOTICE 'temppid2 naplnena';
  CREATE INDEX ON temppid2 (tid);
  EXECUTE 'UPDATE ' || tintable ||' AS a SET pid2 = (SELECT pid2 FROM temppid2 AS b WHERE a.tid = b.tid)';
  RAISE NOTICE 'pid2 updatnute';
  
  CREATE TABLE temppid3 (tid integer, pid3 integer);
  WITH tidpid3 AS (
	 SELECT tid, pid3 FROM temptplg1 WHERE pid3 IS NOT NULL UNION
	 SELECT tid, pid3 FROM temptplg2 WHERE pid3 IS NOT NULL UNION
	 SELECT tid, pid3 FROM temptplg3 WHERE pid3 IS NOT NULL UNION
	 SELECT tid, pid3 FROM temptplg4 WHERE pid3 IS NOT NULL UNION
	 SELECT tid, pid3 FROM temptplg5 WHERE pid3 IS NOT NULL ORDER BY tid
  )
  INSERT INTO temppid3 SELECT * FROM tidpid3;
  RAISE NOTICE 'temppid3 naplnena';
  CREATE INDEX ON temppid3 (tid);
  EXECUTE 'UPDATE ' || tintable ||' AS a SET pid3 = (SELECT pid3 FROM temppid3 AS b WHERE a.tid = b.tid)';
  RAISE NOTICE 'pid3 updatnute';
  
  DROP TABLE temptplg1;
  DROP TABLE temptplg2;
  DROP TABLE temptplg3;
  DROP TABLE temptplg4;
  DROP TABLE temptplg5;
  DROP TABLE temppid1;
  DROP TABLE temppid2;
  DROP TABLE temppid3;
  
  EXECUTE 'CREATE INDEX ON ' || tintable || ' (pid1)';
  EXECUTE 'CREATE INDEX ON ' || tintable || ' (pid2)';
  EXECUTE 'CREATE INDEX ON ' || tintable || ' (pid3)';
  EXECUTE 'ANALYZE ' || tintable;
  
  RETURN 'OK';
END
$$
LANGUAGE plpgsql;   

CREATE OR REPLACE FUNCTION _tin_topology(pidmin integer, pidmax integer, tintable text, connum integer) RETURNS void AS -- Nova od verzie 1.01
$$
DECLARE
  pid integer;
  r RECORD;
  r2 RECORD;
  srid integer;
BEGIN
  EXECUTE 'SELECT ST_SRID(geom) FROM ' || tintable || ' LIMIT 1' INTO srid;
  pid := pidmin;
  FOR r IN EXECUTE 'SELECT geom FROM temppnts WHERE id >= ' || pidmin || ' AND id <= ' || pidmax
  LOOP
    FOR r2 IN EXECUTE 'SELECT tid, geom FROM '|| tintable || ' WHERE ST_DWithin(ST_GeomFromText(' || quote_literal(ST_AsText(r.geom)) || ', ' || srid || '), ST_Boundary(geom), 0.0000001)'
    LOOP
      RAISE NOTICE 'pid je %', pid;
      IF ST_Equals(r.geom, ST_PointN(ST_Boundary(r2.geom), 1)) THEN
        EXECUTE 'INSERT INTO temptplg' || connum::text || ' (tid, pid1) VALUES (' || r2.tid || ', ' || pid || ')';
      ELSIF ST_Equals(r.geom, ST_PointN(ST_Boundary(r2.geom), 2)) THEN 
        EXECUTE 'INSERT INTO temptplg' || connum::text || ' (tid, pid2) VALUES (' || r2.tid || ', ' || pid || ')';
      ELSIF ST_Equals(r.geom, ST_PointN(ST_Boundary(r2.geom), 3)) THEN
        EXECUTE 'INSERT INTO temptplg' || connum::text || ' (tid, pid3) VALUES (' || r2.tid || ', ' || pid || ')'; 
      END IF; 
    END LOOP;
    pid := pid + 1;
  END LOOP;
END
$$
LANGUAGE plpgsql;  

CREATE OR REPLACE FUNCTION tin_topologyntids(tintable text) RETURNS text AS -- Nova od verzie 1.01
$$
DECLARE
  r RECORD;
  i integer;
  j integer;
BEGIN
  EXECUTE 'ALTER TABLE ' || tintable || ' DROP COLUMN IF EXISTS ntid1';
  EXECUTE 'ALTER TABLE ' || tintable || ' DROP COLUMN IF EXISTS ntid2';
  EXECUTE 'ALTER TABLE ' || tintable || ' DROP COLUMN IF EXISTS ntid3';
  EXECUTE 'ALTER TABLE ' || tintable || ' ADD COLUMN ntid1 integer';
  EXECUTE 'ALTER TABLE ' || tintable || ' ADD COLUMN ntid2 integer';
  EXECUTE 'ALTER TABLE ' || tintable || ' ADD COLUMN ntid3 integer';
  
  j := 1;
  FOR r IN EXECUTE 'SELECT tid, pid1, pid2, pid3 FROM ' || tintable
  LOOP
    RAISE NOTICE 'robim %. trojuholnik', j;
    j := j + 1;
    EXECUTE 'SELECT tid FROM ' || tintable || ' WHERE (pid1 = ' || r.pid1 || ' OR pid2 = ' || r.pid1 || ' OR pid3 = ' || r.pid1 || ') AND (pid1 = '
      || r.pid2 || ' OR pid2 = ' || r.pid2 || ' OR pid3 = ' || r.pid2 || ') AND tid <> ' || r.tid INTO i;
    IF i IS NOT NULL THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid3 = ' || i || ' WHERE tid = ' || r.tid;
    ELSE
      EXECUTE 'UPDATE ' || tintable || ' SET ntid3 = -1 WHERE tid = ' || r.tid;
    END IF;
    
    EXECUTE 'SELECT tid FROM ' || tintable || ' WHERE (pid1 = ' || r.pid1 || ' OR pid2 = ' || r.pid1 || ' OR pid3 = ' || r.pid1 || ') AND (pid1 = '
      || r.pid3 || ' OR pid2 = ' || r.pid3 || ' OR pid3 = ' || r.pid3 || ') AND tid <> ' || r.tid INTO i;
    IF i IS NOT NULL THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid2 = ' || i || ' WHERE tid = ' || r.tid;
    ELSE
      EXECUTE 'UPDATE ' || tintable || ' SET ntid2 = -1 WHERE tid = ' || r.tid;
    END IF;
    
    EXECUTE 'SELECT tid FROM ' || tintable || ' WHERE (pid1 = ' || r.pid2 || ' OR pid2 = ' || r.pid2 || ' OR pid3 = ' || r.pid2 || ') AND (pid1 = '
      || r.pid3 || ' OR pid2 = ' || r.pid3 || ' OR pid3 = ' || r.pid3 || ') AND tid <> ' || r.tid INTO i;
    IF i IS NOT NULL THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid1 = ' || i || ' WHERE tid = ' || r.tid;
    ELSE
      EXECUTE 'UPDATE ' || tintable || ' SET ntid1 = -1 WHERE tid = ' || r.tid;
    END IF;
  END LOOP;
  
  RETURN 'OK';
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION tin_flip(tid1 integer, tid2 integer, tintable text) RETURNS text AS -- Upravena od verzie 1.01
$$
DECLARE
  flipped geometry[];
  testtroj geometry;
  testbod geometry;
  testpid integer;
  pomntid integer;
  pida1 integer;
  pida2 integer;
  pida3 integer;
  pidb1 integer;
  pidb2 integer;
  pidb3 integer;
  pids integer[];
  ntida1 integer := -1;
  ntida2 integer := -1;
  ntida3 integer := -1;
  ntidb1 integer := -1;
  ntidb2 integer := -1;
  ntidb3 integer := -1;
  ntids integer[];
  i integer;
  r record;
BEGIN
  EXECUTE 'SELECT Count(*) FROM ' || tintable || ' WHERE ' || tid1 || ' IN (ntid1, ntid2, ntid3) AND tid = ' || tid2 INTO i;
  IF i = 0 THEN
    RAISE EXCEPTION 'Input triangles are not neighbours';
  END IF;
  
  EXECUTE 'SELECT _TIN_Flip(t1.geom, t2.geom) FROM
    (SELECT geom FROM ' || tintable || ' WHERE tid = ' || tid1::text || ') AS t1,(SELECT geom FROM ' || tintable || ' WHERE tid = ' || tid2::text || ') AS t2' INTO flipped;
  EXECUTE 'SELECT array_agg(subsel.pids) FROM (SELECT DISTINCT unnest(ARRAY[pid1, pid2, pid3]) AS pids FROM ' || tintable || 
    ' WHERE tid IN (' || tid1 || ', ' || tid2 || ')) AS subsel' INTO pids;
  FOR i IN 1..array_length(pids, 1)
  LOOP
    testpid := pids[i];
    EXECUTE 'SELECT geom FROM ' || tintable || ' WHERE ' || testpid || ' IN (pid1) LIMIT 1' INTO testtroj;
    IF testtroj IS NOT NULL THEN
      testbod := ST_PointN(ST_Boundary(testtroj), 1);
    ELSE
      EXECUTE 'SELECT geom FROM ' || tintable || ' WHERE ' || testpid || ' IN (pid2) LIMIT 1' INTO testtroj;
      IF testtroj IS NOT NULL THEN
        testbod := ST_PointN(ST_Boundary(testtroj), 2);
      ELSE
        EXECUTE 'SELECT geom FROM ' || tintable || ' WHERE ' || testpid || ' IN (pid3) LIMIT 1' INTO testtroj;
        IF testtroj IS NOT NULL THEN
          testbod := ST_PointN(ST_Boundary(testtroj), 3);
        END IF;
      END IF;
    END IF;
    IF ST_Equals(testbod, ST_PointN(ST_Boundary(flipped[1]), 1)) THEN
      pida1 := testpid;
    ELSIF ST_Equals(testbod, ST_PointN(ST_Boundary(flipped[1]), 2)) THEN
      pida2 := testpid;
    ELSIF ST_Equals(testbod, ST_PointN(ST_Boundary(flipped[1]), 3)) THEN
      pida3 := testpid;
    END IF;
    IF ST_Equals(testbod, ST_PointN(ST_Boundary(flipped[2]), 1)) THEN
      pidb1 := testpid;
    ELSIF ST_Equals(testbod, ST_PointN(ST_Boundary(flipped[2]), 2)) THEN
      pidb2 := testpid;
    ELSIF ST_Equals(testbod, ST_PointN(ST_Boundary(flipped[2]), 3)) THEN
      pidb3 := testpid;
    END IF;
  END LOOP;
  
  EXECUTE 'SELECT ARRAY[a.ntid1, a.ntid2, a.ntid3, b.ntid1, b.ntid2, b.ntid3] FROM 
    (SELECT ntid1, ntid2, ntid3 FROM ' || tintable || ' WHERE tid = ' || tid1 || ') AS a,
    (SELECT ntid1, ntid2, ntid3 FROM ' || tintable || ' WHERE tid = ' || tid2 || ') AS b'
    INTO ntids;
  ntids := array_remove(ntids, tid1);
  ntids := array_remove(ntids, tid2);
  ntids := array_remove(ntids, -1);
  
  FOR i IN 1..array_length(ntids, 1)
  LOOP
    pomntid := ntids[i];
    EXECUTE 'SELECT pid1, pid2, pid3 FROM ' || tintable || ' WHERE tid = ' || pomntid INTO r;
    IF (r.pid1 = pida1 OR r.pid1 = pida2 OR r.pid1 = pida3) AND (r.pid2 = pida1 OR r.pid2 = pida2 OR r.pid2 = pida3) THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid3 = ' || tid1 || ' WHERE tid = ' || pomntid;
    ELSIF (r.pid1 = pida1 OR r.pid1 = pida2 OR r.pid1 = pida3) AND (r.pid3 = pida1 OR r.pid3 = pida2 OR r.pid3 = pida3) THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid2 = ' || tid1 || ' WHERE tid = ' || pomntid;
    ELSIF (r.pid2 = pida1 OR r.pid2 = pida2 OR r.pid2 = pida3) AND (r.pid3 = pida1 OR r.pid3 = pida2 OR r.pid3 = pida3) THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid1 = ' || tid1 || ' WHERE tid = ' || pomntid;
    ELSIF (r.pid1 = pidb1 OR r.pid1 = pidb2 OR r.pid1 = pidb3) AND (r.pid2 = pidb1 OR r.pid2 = pidb2 OR r.pid2 = pidb3) THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid3 = ' || tid2 || ' WHERE tid = ' || pomntid;
    ELSIF (r.pid1 = pidb1 OR r.pid1 = pidb2 OR r.pid1 = pidb3) AND (r.pid3 = pidb1 OR r.pid3 = pidb2 OR r.pid3 = pidb3) THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid2 = ' || tid2 || ' WHERE tid = ' || pomntid;
    ELSIF (r.pid2 = pidb1 OR r.pid2 = pidb2 OR r.pid2 = pidb3) AND (r.pid3 = pidb1 OR r.pid3 = pidb2 OR r.pid3 = pidb3) THEN
      EXECUTE 'UPDATE ' || tintable || ' SET ntid1 = ' || tid2 || ' WHERE tid = ' || pomntid;
    END IF;
    IF (pida1 = r.pid1 OR pida1 = r.pid2 OR pida1 = r.pid3) AND (pida2 = r.pid1 OR pida2 = r.pid2 OR pida2 = r.pid3) THEN
      ntida3 := pomntid;
    ELSIF (pida1 = r.pid1 OR pida1 = r.pid2 OR pida1 = r.pid3) AND (pida3 = r.pid1 OR pida3 = r.pid2 OR pida3 = r.pid3) THEN
      ntida2 := pomntid;
    ELSIF (pida2 = r.pid1 OR pida2 = r.pid2 OR pida2 = r.pid3) AND (pida3 = r.pid1 OR pida3 = r.pid2 OR pida3 = r.pid3) THEN
      ntida1 := pomntid;
    ELSIF (pidb1 = r.pid1 OR pidb1 = r.pid2 OR pidb1 = r.pid3) AND (pidb2 = r.pid1 OR pidb2 = r.pid2 OR pidb2 = r.pid3) THEN
      ntidb3 := pomntid;
    ELSIF (pidb1 = r.pid1 OR pidb1 = r.pid2 OR pidb1 = r.pid3) AND (pidb3 = r.pid1 OR pidb3 = r.pid2 OR pidb3 = r.pid3) THEN
      ntidb2 := pomntid;
    ELSIF (pidb2 = r.pid1 OR pidb2 = r.pid2 OR pidb2 = r.pid3) AND (pidb3 = r.pid1 OR pidb3 = r.pid2 OR pidb3 = r.pid3) THEN
      ntidb1 := pomntid;
    END IF;              
  END LOOP;
  
  IF (pida1 = pidb1 OR pida1 = pidb2 OR pida1 = pidb3) AND (pida2 = pidb1 OR pida2 = pidb2 OR pida2 = pidb3) THEN
    ntida3 := tid2;
  ELSIF (pida1 = pidb1 OR pida1 = pidb2 OR pida1 = pidb3) AND (pida3 = pidb1 OR pida3 = pidb2 OR pida3 = pidb3) THEN
    ntida2 := tid2;
  ELSIF (pida2 = pidb1 OR pida2 = pidb2 OR pida2 = pidb3) AND (pida3 = pidb1 OR pida3 = pidb2 OR pida3 = pidb3) THEN
    ntida1 := tid2;
  END IF;
  IF (pidb1 = pida1 OR pidb1 = pida2 OR pidb1 = pida3) AND (pidb2 = pida1 OR pidb2 = pida2 OR pidb2 = pida3) THEN
    ntidb3 := tid1;
  ELSIF (pidb1 = pida1 OR pidb1 = pida2 OR pidb1 = pida3) AND (pidb3 = pida1 OR pidb3 = pida2 OR pidb3 = pida3) THEN
    ntidb2 := tid1;
  ELSIF (pidb2 = pida1 OR pidb2 = pida2 OR pidb2 = pida3) AND (pidb3 = pida1 OR pidb3 = pida2 OR pidb3 = pida3) THEN
    ntidb1 := tid1;
  END IF; 
  
  EXECUTE 'UPDATE ' || tintable || ' SET geom = ST_GeomFromText(' || quote_literal(ST_AsText(flipped[1])) || '), pid1 = ' || pida1 || ', pid2 = ' || pida2 || ', pid3 = ' || pida3 ||
    ', ntid1 = ' || ntida1 || ', ntid2 = ' || ntida2 || ', ntid3 = ' || ntida3 || ' WHERE tid = ' || tid1;
  EXECUTE 'UPDATE ' || tintable || ' SET geom = ST_GeomFromText(' || quote_literal(ST_AsText(flipped[2])) || '), pid1 = ' || pidb1 || ', pid2 = ' || pidb2 || ', pid3 = ' || pidb3 ||
    ', ntid1 = ' || ntidb1 || ', ntid2 = ' || ntidb2 || ', ntid3 = ' || ntidb3 || ' WHERE tid = ' || tid2;
  
  RETURN 'OK';
END
$$
LANGUAGE plpgsql; 
  
CREATE OR REPLACE FUNCTION tin_pntcommontriangles(tid integer, tintable text, vertexnum integer) RETURNS geometry[] AS  -- Upravena od verzie 1.01
$$
DECLARE
  pid integer;
	vystup geometry[];
BEGIN
  EXECUTE 'SELECT pid' || vertexnum || ' FROM ' || tintable || ' WHERE tid = ' || tid INTO pid;
  EXECUTE 'SELECT array_agg(geom) FROM ' || tintable || ' WHERE pid1 = ' || pid || ' OR pid2 = ' || pid ||' OR pid3 = ' || pid INTO vystup;
	RETURN vystup;
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION tin_getpoint(pid integer, tintable text) RETURNS geometry AS -- Nova od verzie 1.01
$$
DECLARE
  bod geometry;
  max integer;
BEGIN
  EXECUTE 'SELECT ST_PointN(ST_Boundary(geom), 1) FROM ' || tintable || ' WHERE ' || pid || ' IN (pid1) LIMIT 1' INTO bod;
  IF bod IS NULL THEN
    EXECUTE 'SELECT ST_PointN(ST_Boundary(geom), 2) FROM ' || tintable || ' WHERE ' || pid || ' IN (pid2) LIMIT 1' INTO bod;
    IF bod IS NULL THEN
      EXECUTE 'SELECT ST_PointN(ST_Boundary(geom), 3) FROM ' || tintable || ' WHERE ' || pid || ' IN (pid3) LIMIT 1' INTO bod;
      IF bod IS NULL THEN
        EXECUTE 'SELECT max(pids) FROM (SELECT unnest(ARRAY[max(pid1), max(pid2), max(pid3)]) AS pids FROM ' || tintable || ') AS sub' INTO max;
        RAISE EXCEPTION 'No such pid exists. Please use integer values from 1 to %', max;
      END IF;
    END IF;
  END IF;
  RETURN bod;
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION tin_partialderivativesonvertex(pid integer, tintable text) RETURNS double precision[] AS  -- Upravena od verzie 1.01
$$
DECLARE
  poletroj geometry[];
  bod geometry;
BEGIN
  bod := TIN_GetPoint(pid, tintable);
  EXECUTE 'SELECT array_agg(geom) FROM ' || tintable || ' WHERE ' || pid || ' IN (pid1, pid2, pid3)' INTO poletroj;
  RETURN _TIN_PartialDerivativesOnVertex(bod, poletroj);  
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION tin_slopeonvertex(pid integer, tintable text) RETURNS double precision AS  -- Upravena od verzie 1.01
$$
DECLARE
  poletroj geometry[];
  bod geometry;
BEGIN
  bod := TIN_GetPoint(pid, tintable);
  EXECUTE 'SELECT array_agg(geom) FROM ' || tintable || ' WHERE ' || pid || ' IN (pid1, pid2, pid3)' INTO poletroj;
  RETURN _TIN_SlopeOnVertex(bod, poletroj);  
END
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION tin_aspectonvertex(pid integer, tintable text) RETURNS double precision AS  -- Upravena od verzie 1.01
$$
DECLARE
  poletroj geometry[];
  bod geometry;
BEGIN
  bod := TIN_GetPoint(pid, tintable);
  EXECUTE 'SELECT array_agg(geom) FROM ' || tintable || ' WHERE ' || pid || ' IN (pid1, pid2, pid3)' INTO poletroj;
  RETURN _TIN_AspectOnVertex(bod, poletroj);  
END
$$
LANGUAGE plpgsql;
  
CREATE OR REPLACE FUNCTION tin_area3d(triangle geometry)
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_Area3D'
  LANGUAGE c STRICT;
  
CREATE OR REPLACE FUNCTION tin_centroid(triangle geometry)
  RETURNS geometry AS
'$libdir/pg3angles-1.01', 'TIN_Centroid'
  LANGUAGE c STRICT;
  
CREATE OR REPLACE FUNCTION tin_partialderivativesoftriangle(triangle geometry)
  RETURNS double precision[] AS
'$libdir/pg3angles-1.01', 'TIN_PartialDerivativesOfTriangle'
  LANGUAGE c STRICT;
  
CREATE OR REPLACE FUNCTION tin_slopeoftriangle(triangle geometry)
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_SlopeOfTriangle'
  LANGUAGE c STRICT;
  
CREATE OR REPLACE FUNCTION tin_aspectoftriangle(triangle geometry)
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_AspectOfTriangle'
  LANGUAGE c STRICT;
  
CREATE OR REPLACE FUNCTION tin_linearz(triangle geometry, point geometry)
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_LinearZ'
  LANGUAGE c STRICT;

CREATE OR REPLACE FUNCTION _tin_flip(triangle1 geometry, triangle2 geometry)
  RETURNS geometry[] AS
'$libdir\pg3angles-1.01', 'TIN_Flip'
	LANGUAGE c STRICT; 
  
CREATE OR REPLACE FUNCTION _tin_partialderivativesonvertex(point geometry, triangles geometry[])
  RETURNS double precision[] AS
'$libdir/pg3angles-1.01', 'TIN_PartialDerivativesOnVertex'
  LANGUAGE c STRICT;  

CREATE OR REPLACE FUNCTION _tin_slopeonvertex(point geometry, triangles geometry[])
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_SlopeOnVertex'
  LANGUAGE c STRICT;
  
CREATE OR REPLACE FUNCTION _tin_aspectonvertex(point geometry, triangles geometry[])
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_AspectOnVertex'
  LANGUAGE c STRICT;

CREATE OR REPLACE FUNCTION tin_quasiquadraticz(triangle geometry, triangles1 geometry[], triangles2 geometry[], triangles3 geometry[], point geometry)
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_QuasiQuadraticZ'
  LANGUAGE c STRICT;

CREATE OR REPLACE FUNCTION tin_quasiquadraticderivatives(triangle geometry, triangles1 geometry[], triangles2 geometry[], triangles3 geometry[], point geometry)
  RETURNS double precision[] AS
'$libdir/pg3angles-1.01', 'TIN_QuasiQuadraticDerivatives'
  LANGUAGE c STRICT;

CREATE OR REPLACE FUNCTION tin_normalsangle(normal1 double precision[], normal2 double precision[])
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_NormalsAngle'
  LANGUAGE c  STRICT;
  
CREATE OR REPLACE FUNCTION tin_isconvexhull(triangle1 geometry, triangle2 geometry)
  RETURNS boolean AS
'$libdir/pg3angles-1.01', 'TIN_IsConvexHull'
  LANGUAGE c STRICT;
  
CREATE OR REPLACE FUNCTION tin_contours(triangle geometry, deltaz double precision)
  RETURNS geometry[] AS
'$libdir/pg3angles-1.01', 'TIN_Contours'
  LANGUAGE c STRICT;
  
CREATE OR REPLACE FUNCTION tin_cubature(triangle geometry, basez double precision, inverse boolean DEFAULT false)
  RETURNS double precision AS
'$libdir/pg3angles-1.01', 'TIN_Cubature'
  LANGUAGE c STRICT;