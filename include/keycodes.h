#ifndef TERRACOTTA_KEYCODES_H
#define TERRACOTTA_KEYCODES_H

#define KEY_NULL        0x00

/* Basic symbols */
#define KEY_0           0x30
#define KEY_1           0x31
#define KEY_2           0x32
#define KEY_3           0x33
#define KEY_4           0x34
#define KEY_5           0x35
#define KEY_6           0x36
#define KEY_7           0x37
#define KEY_8           0x38
#define KEY_9           0x39

#define KEY_A           0x61
#define KEY_B           0x62
#define KEY_C           0x63
#define KEY_D           0x64
#define KEY_E           0x65
#define KEY_F           0x66
#define KEY_G           0x67
#define KEY_H           0x68
#define KEY_I           0x69
#define KEY_J           0x6A
#define KEY_K           0x6B
#define KEY_L           0x6C
#define KEY_M           0x6D
#define KEY_N           0x6E
#define KEY_O           0x6F
#define KEY_P           0x70
#define KEY_Q           0x71
#define KEY_R           0x72
#define KEY_S           0x73
#define KEY_T           0x74
#define KEY_U           0x75
#define KEY_V           0x76
#define KEY_W           0x77
#define KEY_X           0x78
#define KEY_Y           0x79
#define KEY_Z           0x7A

/* Shift symbols */
#define KEY_SHIFT_0     KEY_0
#define KEY_SHIFT_1     KEY_1
#define KEY_SHIFT_2     KEY_2
#define KEY_SHIFT_3     KEY_3
#define KEY_SHIFT_4     KEY_4
#define KEY_SHIFT_5     KEY_5
#define KEY_SHIFT_6     KEY_6
#define KEY_SHIFT_7     KEY_7
#define KEY_SHIFT_8     KEY_8
#define KEY_SHIFT_9     KEY_9

#define KEY_SHIFT_A     0x41
#define KEY_SHIFT_B     0x42
#define KEY_SHIFT_C     0x43
#define KEY_SHIFT_D     0x44
#define KEY_SHIFT_E     0x45
#define KEY_SHIFT_F     0x46
#define KEY_SHIFT_G     0x47
#define KEY_SHIFT_H     0x48
#define KEY_SHIFT_I     0x49
#define KEY_SHIFT_J     0x4A
#define KEY_SHIFT_K     0x4B
#define KEY_SHIFT_L     0x4C
#define KEY_SHIFT_M     0x4D
#define KEY_SHIFT_N     0x4E
#define KEY_SHIFT_O     0x4F
#define KEY_SHIFT_P     0x50
#define KEY_SHIFT_Q     0x51
#define KEY_SHIFT_R     0x52
#define KEY_SHIFT_S     0x53
#define KEY_SHIFT_T     0x54
#define KEY_SHIFT_U     0x55
#define KEY_SHIFT_V     0x56
#define KEY_SHIFT_W     0x57
#define KEY_SHIFT_X     0x58
#define KEY_SHIFT_Y     0x59
#define KEY_SHIFT_Z     0x5A

/* Symbols */
#define KEY_SYM_1       0x21 /* ! */
#define KEY_SYM_2       0x40 /* @ */
#define KEY_SYM_3       0x23 /* # */
#define KEY_SYM_4       0x24 /* $ */
#define KEY_SYM_5       0x25 /* % */
#define KEY_SYM_6       0x5E /* ^ */
#define KEY_SYM_7       0x26 /* & */
#define KEY_SYM_8       0x2A /* * */
#define KEY_SYM_9       0x28 /* ( */
#define KEY_SYM_0       0x29 /* ) */

#define KEY_SYM_A       0x3B /* ; */
#define KEY_SYM_B       KEY_NULL
#define KEY_SYM_C       KEY_NULL 
#define KEY_SYM_D       0x60 /* ` */
#define KEY_SYM_E       0x5B /* [ */
#define KEY_SYM_F       0x2B /* + */
#define KEY_SYM_G       0x2D /* - */
#define KEY_SYM_H       0x5F /* _ */
#define KEY_SYM_I       0x7E /* ~ */
#define KEY_SYM_J       0x3D /* = */
#define KEY_SYM_K       0x3F /* ? */
#define KEY_SYM_L       KEY_NULL
#define KEY_SYM_M       KEY_NULL
#define KEY_SYM_N       KEY_NULL
#define KEY_SYM_O       0x27 /* ' */
#define KEY_SYM_P       0x22 /* " */
#define KEY_SYM_Q       0x7B /* { */
#define KEY_SYM_R       0x5D /* ] */
#define KEY_SYM_S       0x3A /* : */
#define KEY_SYM_T       0x2F /* / */
#define KEY_SYM_U       0x7C /* | */
#define KEY_SYM_V       KEY_NULL
#define KEY_SYM_W       0x7D /* } */
#define KEY_SYM_X       KEY_NULL
#define KEY_SYM_Y       0x5C /* \ */
#define KEY_SYM_Z       KEY_NULL

/* FN buttons */
#define KEY_FN_0        0x8A
#define KEY_FN_1        0x81
#define KEY_FN_2        0x82
#define KEY_FN_3        0x83
#define KEY_FN_4        0x84
#define KEY_FN_5        0x85
#define KEY_FN_6        0x86
#define KEY_FN_7        0x87
#define KEY_FN_8        0x88
#define KEY_FN_9        0x89

#define KEY_FN_A        0x9A
#define KEY_FN_B        0xAA
#define KEY_FN_C        0xA8
#define KEY_FN_D        0x9C
#define KEY_FN_E        0x8F
#define KEY_FN_F        0x9D
#define KEY_FN_G        0x9E
#define KEY_FN_H        0x9F
#define KEY_FN_I        0x94
#define KEY_FN_J        0xA0
#define KEY_FN_K        0xA1
#define KEY_FN_L        0xA2
#define KEY_FN_M        0xAC
#define KEY_FN_N        0xAB
#define KEY_FN_O        0x95
#define KEY_FN_P        0x96
#define KEY_FN_Q        0x8D
#define KEY_FN_R        0x90
#define KEY_FN_S        0x9B
#define KEY_FN_T        0x91
#define KEY_FN_U        0x93
#define KEY_FN_V        0xA9
#define KEY_FN_W        0x8E
#define KEY_FN_X        0xA7
#define KEY_FN_Y        0x92
#define KEY_FN_Z        0xA6

/* Controls */
#define KEY_ESC         0x1B
#define KEY_TAB         0x09
#define KEY_BACK        0x08
#define KEY_SPACE       0x20
#define KEY_ENTER       0x0D
#define KEY_SHIFT_BACK  0x7F

#define KEY_FN_ESC      0x80
#define KEY_FN_TAB      0x8C
#define KEY_FN_BACK     0x8B
#define KEY_FN_SPACE    0xAF
#define KEY_FN_ENTER    0xA3

#define KEY_LEFT        0xB4
#define KEY_UP          0xB5
#define KEY_DOWN        0xB6
#define KEY_RIGHT       0xB7

#define KEY_FN_LEFT     0x98
#define KEY_FN_UP       0x99
#define KEY_FN_DOWN     0xA4
#define KEY_FN_RIGHT    0xA5

#endif