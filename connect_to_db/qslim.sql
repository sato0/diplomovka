CREATE OR REPLACE FUNCTION qslim(cstring, cstring, integer, integer, integer, double precision)
  RETURNS text AS
'$libdir/qslim.so', 'qslim'
  LANGUAGE C STRICT;


SELECT qslim('/home/sato/Desktop/dp/connect_to_db/teddy.obj','teddy345.obj',345,3,1,1000.0);