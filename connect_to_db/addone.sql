CREATE OR REPLACE FUNCTION add_one(integer) RETURNS integer
     AS '$libdir/addonePG.so', 'add_one'
     LANGUAGE C STRICT;

-- note overloading of SQL function name "add_one"
CREATE OR REPLACE FUNCTION add_one(double precision) RETURNS double precision
     AS '$libdir/addonePG.so', 'add_one_float8'
     LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION makepoint(point, point) RETURNS point
     AS '$libdir/addonePG.so', 'makepoint'
     LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION copytext(text) RETURNS text
     AS '$libdir/addonePG.so', 'copytext'
     LANGUAGE C STRICT;

CREATE OR REPLACE FUNCTION concat_text(text, text) RETURNS text
     AS '$libdir/addonePG.so', 'concat_text'
     LANGUAGE C STRICT;

 DROP FUNCTION merge2filesPG_func( text,  text);

CREATE OR REPLACE FUNCTION merge2filesPG_func(text, text)
  RETURNS text AS
'$libdir/addonePG.so', 'merge2filesPG_func'
  LANGUAGE c STRICT;
