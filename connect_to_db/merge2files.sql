
CREATE OR REPLACE FUNCTION merge2filesPG_func(text, text)
  RETURNS text AS
'$libdir/merge2filesPG.so', 'merge2filesPG_func'
  LANGUAGE c STRICT;


select merge2filesPG_func('/home/sato/Desktop/dp/connectDP/first.obj', '/home/sato/Desktop/dp/connectDP/second.obj');
