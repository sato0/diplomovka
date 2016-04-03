

ALTER TABLE tin ADD COLUMN aspect numeric(6,2);
  UPDATE tin
    SET aspect = TIN_AspectOfTriangle(geom);

ALTER TABLE tin ADD COLUMN slope numeric(7,2);
  UPDATE tin
    SET slope = TIN_SlopeOfTriangle(geom);

-- REMOVE INVALID TRIANGLES DETECTED VIA SLOPE=NAN
-- DELETE FROM tin WHERE slope = 'NAN';


ALTER TABLE tin ADD COLUMN centroid geometry;
  UPDATE tin
    SET centroid = TIN_Centroid(geom);

ALTER TABLE tin ADD COLUMN centroid_wkt text;
  UPDATE tin
    SET centroid_wkt = ST_AsText(centroid);

ALTER TABLE tin ADD COLUMN area numeric(10,2);
  UPDATE tin
    SET area = TIN_Area3D(geom);

ALTER TABLE tin ADD COLUMN cubature numeric(10,2);
  UPDATE tin
    SET cubature = TIN_Cubature(geom,550);

-- NEIDE TIN_LinearZ
ALTER TABLE tin ADD COLUMN height numeric(10,2);
  UPDATE tin
    SET height = TIN_LinearZ(geom,centroid);


SELECT TIN_TopologyPIDs('tin', 'mariana','mariana', 'dp');
SELECT TIN_TopologyNTIDs('tin');

ALTER TABLE tin ADD COLUMN contours geometry[];
  UPDATE tin
    SET contours = TIN_Contours(geom, 10.0);

ALTER TABLE tin ADD COLUMN partialderivations double precision[];
  UPDATE tin
    SET partialderivations = TIN_PartialDerivativesOfTriangle(geom);

ALTER TABLE tin ADD COLUMN common3on1stvertex geometry[];
  UPDATE tin
    SET common3on1stvertex = TIN_PntCommonTriangles(tid,'tin', 1);




-- double precision[] TIN_PartialDerivativesOnVertex(pid
-- integer, tintable text);

-- double precision TIN_SlopeOnVertex(pid integer,
-- tintable text);

-- double precision TIN_AspectOnVertex(pid integer,
-- tintable text);
-- ntid1,ntid2,ntid3, common3on1stvertex
SELECT tid,aspect,slope,area,cubature,height FROM tin where slope='NaN' or aspect='NaN';

SELECT tid,aspect,slope,area,cubature,height,pid1,pid2,pid3,ntid1,ntid2,ntid3 FROM tin limit 15;
-- DELETE FROM tin WHERE slope = 'NAN';