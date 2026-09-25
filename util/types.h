#ifndef TYPES_H
#define TYPES_H

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* Project type vocabulary. Typedefs are codegen-neutral; the underlying type
   (signed/unsigned, char/int, float/double) is what affects /Od codegen, so the
   width and signedness are explicit here. */

typedef signed char MechS8;
typedef unsigned char MechU8;
typedef short MechS16;
typedef unsigned short MechU16;
typedef int MechS32;
typedef unsigned int MechU32;
typedef float MechFloat;
typedef double MechDouble;
typedef char MechChar;

/* Boolean typedefs (MechBool / MechBool8 ...) are added when a match proves
   the width and signedness. */

#endif /* TYPES_H */
