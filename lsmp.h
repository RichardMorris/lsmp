/*
 * lsmp.h	constants used by lsmp programs 
 *
 * copyright Richard Morris 1996
 */

/* dimensions >0 is just a simple dimension R^n -> R^n */

#define THREE_D_EQN 3
#define FOUR_D_EQN 4
#define THREE_FOUR_D_EQN -3
#define FOUR_THREE_D_EQN -4

/* The posible types of normal transformation */

#define NO_NORM 0
#define STD_NORM 1
#define DISC_NORM 2
#define EQN_NORM 3
#define NO_NORM_NAME "None"
#define STD_NORM_NAME "Std"
#define DISC_NORM_NAME "Disc"
#define EQN_NORM_NAME "Eqn"

/* Colours for curve/surface avoid clash with 0-7 for black-white */
/* note this is incompatable with those used in mapping */

#define NO_COL -1
#define EQN_COL -2
#define STD_COL 0
#define NO_COL_NAME "None"
#define EQN_COL_NAME "Eqn"
#define STD_COL_NAME "Std"

/*Names for options */

#define DIM_NAME "dimension"
#define NORM_NAME "normals"
#define COL_NAME "colour"
#define PREC_NAME "precision"
#define CLIP_NAME "clipping"
#define MODE_NAME "mode"
#define STEPS1_NAME "steps1"
#define STEPS2_NAME "steps2"
#define STEPS_NAME "steps"
#define TOL_NAME "tolerance"
#define ITT_NAME  "itterations"
#define NUM_STEPS_NAME "num_steps"
#define STEP_LEN_NAME "step_len"
#define ORIENT_NAME "oriented"

#define PSDEF_NAME "LSMP_DEF"
#define PSETIME_NAME "LSMP_EDIT_TIMESTAMP"

/* comments */

#define LSMP_DEF_NAME "LSMP_DEF"
#define LSMP_EDIT_TIME_NAME "LSMP_EDIT_TIMESTAMP"

/* Modes */

#define MODE_SIMPLE 0
#define MODE_PSURF 1
#define MODE_ASURF 2
#define MODE_PCURVE 3
#define MODE_ACURVE 4
#define MODE_ACURVE3 5
#define MODE_INTERSECT 6
#define MODE_ICURVE 7
#define MODE_IMPSURF 8
#define MODE_KNOWN_SING 9
#define MODE_DESCRIM2 10
#define MODE_DESCRIM3 11

#define MODE_ICV_PSURF -1
#define MODE_PSURF_PROJ -2
#define MODE_ICV_PSURF_PROJ -3

/* Orientations */

#define ORIENT_UN 0
#define ORIENT_ORIENT 1
#define ORIENT_MAJOR 2
#define ORIENT_MINOR 3

/* Some (but not all) constants */

#define CLIP_DEFAULT 1000.0
#define PREC_DEFAULT 1

typedef struct { double x,y; } VEC2D;
typedef struct { double x,y,z; } VEC3D;



