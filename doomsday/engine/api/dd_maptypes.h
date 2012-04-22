/* Generated by .\..\..\engine\scripts\makedmt.py */

#ifndef __DOOMSDAY_PLAY_PUBLIC_MAP_DATA_TYPES_H__
#define __DOOMSDAY_PLAY_PUBLIC_MAP_DATA_TYPES_H__

#define DMT_VERTEX_ORIGIN DDVT_DOUBLE

#define DMT_HEDGE_SIDEDEF DDVT_PTR

#define DMT_HEDGE_V DDVT_PTR             // [Start, End] of the segment.
#define DMT_HEDGE_LINEDEF DDVT_PTR
#define DMT_HEDGE_SECTOR DDVT_PTR
#define DMT_HEDGE_BSPLEAF DDVT_PTR
#define DMT_HEDGE_TWIN DDVT_PTR
#define DMT_HEDGE_ANGLE DDVT_ANGLE
#define DMT_HEDGE_SIDE DDVT_BYTE         // 0=front, 1=back
#define DMT_HEDGE_LENGTH DDVT_DOUBLE     // Accurate length of the segment (v1 -> v2).
#define DMT_HEDGE_OFFSET DDVT_DOUBLE
#define DMT_HEDGE_NEXT DDVT_PTR
#define DMT_HEDGE_PREV DDVT_PTR

#define DMT_BSPLEAF_HEDGECOUNT DDVT_UINT
#define DMT_BSPLEAF_HEDGE DDVT_PTR
#define DMT_BSPLEAF_POLYOBJ DDVT_PTR // NULL, if there is no polyobj.
#define DMT_BSPLEAF_SECTOR DDVT_PTR

#define DMT_MATERIAL_FLAGS DDVT_SHORT
#define DMT_MATERIAL_WIDTH DDVT_INT
#define DMT_MATERIAL_HEIGHT DDVT_INT

#define DMT_SURFACE_BASE DDVT_PTR
#define DMT_SURFACE_FLAGS DDVT_INT     // SUF_ flags
#define DMT_SURFACE_MATERIAL DDVT_PTR
#define DMT_SURFACE_BLENDMODE DDVT_BLENDMODE
#define DMT_SURFACE_BITANGENT DDVT_FLOAT
#define DMT_SURFACE_TANGENT DDVT_FLOAT
#define DMT_SURFACE_NORMAL DDVT_FLOAT
#define DMT_SURFACE_OFFSET DDVT_FLOAT     // [X, Y] Planar offset to surface material origin.
#define DMT_SURFACE_RGBA DDVT_FLOAT    // Surface color tint

#define DMT_PLANE_SECTOR DDVT_PTR      // Owner of the plane (temp)
#define DMT_PLANE_HEIGHT DDVT_DOUBLE   // Current height
#define DMT_PLANE_GLOW DDVT_FLOAT      // Glow amount
#define DMT_PLANE_GLOWRGB DDVT_FLOAT   // Glow color
#define DMT_PLANE_TARGET DDVT_DOUBLE   // Target height
#define DMT_PLANE_SPEED DDVT_DOUBLE    // Move speed

#define DMT_SECTOR_FLOORPLANE DDVT_PTR
#define DMT_SECTOR_CEILINGPLANE DDVT_PTR

#define DMT_SECTOR_VALIDCOUNT DDVT_INT // if == validCount, already checked.
#define DMT_SECTOR_LIGHTLEVEL DDVT_FLOAT
#define DMT_SECTOR_RGB DDVT_FLOAT
#define DMT_SECTOR_MOBJLIST DDVT_PTR   // List of mobjs in the sector.
#define DMT_SECTOR_LINEDEFCOUNT DDVT_UINT
#define DMT_SECTOR_LINEDEFS DDVT_PTR   // [lineDefCount+1] size.
#define DMT_SECTOR_BSPLEAFCOUNT DDVT_UINT
#define DMT_SECTOR_BSPLEAFS DDVT_PTR   // [bspLeafCount+1] size.
#define DMT_SECTOR_BASE DDVT_PTR
#define DMT_SECTOR_PLANECOUNT DDVT_UINT
#define DMT_SECTOR_REVERB DDVT_FLOAT

#define DMT_SIDEDEF_LINE DDVT_PTR
#define DMT_SIDEDEF_SECTOR DDVT_PTR
#define DMT_SIDEDEF_FLAGS DDVT_SHORT

#define DMT_LINEDEF_SEC DDVT_PTR
#define DMT_LINEDEF_AABOX DDVT_DOUBLE

#define DMT_LINEDEF_V DDVT_PTR
#define DMT_LINEDEF_SIDEDEFS DDVT_PTR
#define DMT_LINEDEF_FLAGS DDVT_INT     // Public DDLF_* flags.
#define DMT_LINEDEF_SLOPETYPE DDVT_INT
#define DMT_LINEDEF_VALIDCOUNT DDVT_INT
#define DMT_LINEDEF_DX DDVT_DOUBLE
#define DMT_LINEDEF_DY DDVT_DOUBLE

#define DMT_BSPNODE_AABOX DDVT_DOUBLE

#define DMT_BSPNODE_CHILDREN DDVT_PTR

#endif
