#!/bin/bash

#DEFAULT VALUES OF VARIABLES (YOU MAY CHANGE VALUES AND DON'T USE COMMANDLINE OPTIONS)
XXXPG_USER_D='mariana'
DATABASE_D='dp'
PORT_D='5432'
HOST_D='localhost'
PASSWORD_D='mariana'
TEMP_FILE_NAME_D='basicTIN'
INPUT_FILE_PATH_D='/home/sato/Desktop/dp/las_data_samples/clip_txt/ct2495smaller4.csv'
WORK_DIR_D='/home/sato/Desktop/dp/connect_to_db/'
SRID_D='21781'
#QSLIM OPTIMIZATION PARAMETERS
FACE_COUNT_D=1000
EDGE_CONTRACTION_POLICY_D=3
#IMPORTANT KEEP THIS PARAMETER VALUE 0, OTHERWISE QSLIM CREATES TIN WITH DUPLICATE GEOMETRY !!!!!!!!!!!!!1
WEIGHT_D=0	
QUADRIC_WEIGHTING_POLICY_D=1

#COMMANDLINE OPTIONS PROCESSING
while getopts 'u:d:p:h:P:t:i:w:s:o:F:E:W:Q:' flag; do
	case "$flag" in
		u) XXXPG_USER="${OPTARG}" ;;
		d) DATABASE="${OPTARG}" ;;
		p) PORT="${OPTARG}" ;;
		h) HOST="${OPTARG}" ;;
		P) PASSWORD="${OPTARG}" ;;
		t) TEMP_FILE_NAME="${OPTARG}" ;;
		i) INPUT_FILE_PATH="${OPTARG}" ;;		
		w) WORK_DIR="${OPTARG}" ;;
		s) SRID="${OPTARG}" ;;
		F) FACE_COUNT="${OPTARG}" ;;
		E) EDGE_CONTRACTION_POLICY="${OPTARG}" ;;
		W) WEIGHT="${OPTARG}" ;;
		Q) QUADRIC_WEIGHTING_POLICY="${OPTARG}" ;;
		*) error "Unexpected option ${flag}" ;;
	esac
done

#DEFAULT VALUES SUBSTITUTION
XXXPG_USER="${XXXPG_USER:=$XXXPG_USER_D}"
DATABASE="${DATABASE:=$DATABASE_D}"
PORT="${PORT:=$PORT_D}"
HOST="${HOST:=$HOST_D}"
PASSWORD="${PASSWORD:=$PASSWORD_D}"
TEMP_FILE_NAME="${TEMP_FILE_NAME:=$TEMP_FILE_NAME_D}"
INPUT_FILE_PATH="${INPUT_FILE_PATH:=$INPUT_FILE_PATH_D}"
WORK_DIR="${WORK_DIR:=$WORK_DIR_D}"
SRID="${SRID:=$SRID_D}"
FACE_COUNT="${FACE_COUNT:=$FACE_COUNT_D}"
EDGE_CONTRACTION_POLICY="${EDGE_CONTRACTION_POLICY:=$EDGE_CONTRACTION_POLICY_D}"
WEIGHT="${WEIGHT:=$WEIGHT_D}"
QUADRIC_WEIGHTING_POLICY="${QUADRIC_WEIGHTING_POLICY:=$QUADRIC_WEIGHTING_POLICY_D}"

echo "#################################" $XXXPG_USER $DATABASE $PORT $HOST $PASSWORD $TEMP_FILE_NAME $INPUT_FILE_PATH $WORK_DIR $SRID $FACE_COUNT $EDGE_CONTRACTION_POLICY $QUADRIC_WEIGHTING_POLICY

#remove old temporal generated files
rm -f optimizedTIN.obj
rm -f $TEMP_FILE_NAME.obj

START=$(date +%s.%N)

#create postgres functions 
psql -U $XXXPG_USER -d $DATABASE -h $HOST -f "$WORK_DIR"functions.sql 
#load xyz data file, execute Delaunay triangulation and convert to obj format, write into file (qslim input)
psql -U $XXXPG_USER -d $DATABASE -h $HOST -c "SELECT triangulation_for_qslim('$INPUT_FILE_PATH', '$WORK_DIR', '$TEMP_FILE_NAME')"
#optimization
echo "QSLIM OPTIMIZATION START"
./qslim -O "$EDGE_CONTRACTION_POLICY" -B "$WEIGHT" -W "$QUADRIC_WEIGHTING_POLICY" -t "$FACE_COUNT" -o optimizedTIN.obj $TEMP_FILE_NAME.obj
echo "QSLIM OPTIMIZATION END"

#load optimized TIN in database and prepare format for pq3angles
psql -U $XXXPG_USER -d $DATABASE -h $HOST -c "SELECT TIN_for_pg3angles('"$WORK_DIR"optimizedTIN.obj','$SRID')"

psql -U $XXXPG_USER -d $DATABASE -h $HOST -f "$WORK_DIR"pg3angles_examples.sql

END=$(date +%s.%N)
DIFF=$(echo "$END - $START" | bc)
echo $DIFF

#  Customizing simplification:

# -O <n>

# 	Specify the policy for selecting the target position
# 	for edge contractions.  The following levels of
# 	optimization are available:

# 		3 -- Pick point which minimizes error [default].
# 		2 -- Pick best point along edge.
# 		1 -- Pick best of endpoints or midpoint.
# 		0 -- Pick best of the two endpoints.

# -B <weight>

# 	Specifies the weight assigned to boundary constraint
# 	planes.  The default value is currently 1000.  Specify
# 	a weight of 0 to disable boundary constraints entirely.

# -W <n>

# 	Select the quadric weighting policy.  Available policies
# 	are:

# 		0 -- Weight all quadrics uniformly
# 		1 -- Weight by area of contributing triangle [default]
# 		2 -- Weight by angle around vertex
