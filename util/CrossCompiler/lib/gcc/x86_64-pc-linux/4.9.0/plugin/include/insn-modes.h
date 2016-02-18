/* Generated automatically from machmode.def and config/i386/i386-modes.def
   by genmodes.  */

#ifndef GCC_INSN_MODES_H
#define GCC_INSN_MODES_H

enum machine_mode
{
  VOIDmode,                /* machmode.def:172 */
  BLKmode,                 /* machmode.def:176 */
  CCmode,                  /* machmode.def:207 */
  CCGCmode,                /* config/i386/i386-modes.def:61 */
  CCGOCmode,               /* config/i386/i386-modes.def:62 */
  CCNOmode,                /* config/i386/i386-modes.def:63 */
  CCAmode,                 /* config/i386/i386-modes.def:64 */
  CCCmode,                 /* config/i386/i386-modes.def:65 */
  CCOmode,                 /* config/i386/i386-modes.def:66 */
  CCSmode,                 /* config/i386/i386-modes.def:67 */
  CCZmode,                 /* config/i386/i386-modes.def:68 */
  CCFPmode,                /* config/i386/i386-modes.def:69 */
  CCFPUmode,               /* config/i386/i386-modes.def:70 */
  BImode,                  /* machmode.def:179 */
  QImode,                  /* machmode.def:187 */
  HImode,                  /* machmode.def:188 */
  SImode,                  /* machmode.def:189 */
  DImode,                  /* machmode.def:190 */
  TImode,                  /* machmode.def:191 */
  OImode,                  /* config/i386/i386-modes.def:90 */
  XImode,                  /* config/i386/i386-modes.def:91 */
  QQmode,                  /* machmode.def:210 */
  HQmode,                  /* machmode.def:211 */
  SQmode,                  /* machmode.def:212 */
  DQmode,                  /* machmode.def:213 */
  TQmode,                  /* machmode.def:214 */
  UQQmode,                 /* machmode.def:216 */
  UHQmode,                 /* machmode.def:217 */
  USQmode,                 /* machmode.def:218 */
  UDQmode,                 /* machmode.def:219 */
  UTQmode,                 /* machmode.def:220 */
  HAmode,                  /* machmode.def:222 */
  SAmode,                  /* machmode.def:223 */
  DAmode,                  /* machmode.def:224 */
  TAmode,                  /* machmode.def:225 */
  UHAmode,                 /* machmode.def:227 */
  USAmode,                 /* machmode.def:228 */
  UDAmode,                 /* machmode.def:229 */
  UTAmode,                 /* machmode.def:230 */
  SFmode,                  /* machmode.def:202 */
  DFmode,                  /* machmode.def:203 */
  XFmode,                  /* config/i386/i386-modes.def:24 */
  TFmode,                  /* config/i386/i386-modes.def:25 */
  SDmode,                  /* machmode.def:242 */
  DDmode,                  /* machmode.def:243 */
  TDmode,                  /* machmode.def:244 */
  CQImode,                 /* machmode.def:238 */
  CHImode,                 /* machmode.def:238 */
  CSImode,                 /* machmode.def:238 */
  CDImode,                 /* machmode.def:238 */
  CTImode,                 /* machmode.def:238 */
  COImode,                 /* machmode.def:238 */
  CXImode,                 /* machmode.def:238 */
  SCmode,                  /* machmode.def:239 */
  DCmode,                  /* machmode.def:239 */
  XCmode,                  /* machmode.def:239 */
  TCmode,                  /* machmode.def:239 */
  V2QImode,                /* config/i386/i386-modes.def:88 */
  V4QImode,                /* config/i386/i386-modes.def:74 */
  V2HImode,                /* config/i386/i386-modes.def:74 */
  V1SImode,                /* config/i386/i386-modes.def:87 */
  V8QImode,                /* config/i386/i386-modes.def:75 */
  V4HImode,                /* config/i386/i386-modes.def:75 */
  V2SImode,                /* config/i386/i386-modes.def:75 */
  V1DImode,                /* config/i386/i386-modes.def:86 */
  V16QImode,               /* config/i386/i386-modes.def:76 */
  V8HImode,                /* config/i386/i386-modes.def:76 */
  V4SImode,                /* config/i386/i386-modes.def:76 */
  V2DImode,                /* config/i386/i386-modes.def:76 */
  V1TImode,                /* config/i386/i386-modes.def:85 */
  V32QImode,               /* config/i386/i386-modes.def:77 */
  V16HImode,               /* config/i386/i386-modes.def:77 */
  V8SImode,                /* config/i386/i386-modes.def:77 */
  V4DImode,                /* config/i386/i386-modes.def:77 */
  V2TImode,                /* config/i386/i386-modes.def:77 */
  V64QImode,               /* config/i386/i386-modes.def:78 */
  V32HImode,               /* config/i386/i386-modes.def:78 */
  V16SImode,               /* config/i386/i386-modes.def:78 */
  V8DImode,                /* config/i386/i386-modes.def:78 */
  V4TImode,                /* config/i386/i386-modes.def:78 */
  V128QImode,              /* config/i386/i386-modes.def:79 */
  V64HImode,               /* config/i386/i386-modes.def:79 */
  V32SImode,               /* config/i386/i386-modes.def:79 */
  V16DImode,               /* config/i386/i386-modes.def:79 */
  V8TImode,                /* config/i386/i386-modes.def:79 */
  V2SFmode,                /* config/i386/i386-modes.def:80 */
  V4SFmode,                /* config/i386/i386-modes.def:81 */
  V2DFmode,                /* config/i386/i386-modes.def:81 */
  V8SFmode,                /* config/i386/i386-modes.def:82 */
  V4DFmode,                /* config/i386/i386-modes.def:82 */
  V2TFmode,                /* config/i386/i386-modes.def:82 */
  V16SFmode,               /* config/i386/i386-modes.def:83 */
  V8DFmode,                /* config/i386/i386-modes.def:83 */
  V4TFmode,                /* config/i386/i386-modes.def:83 */
  V32SFmode,               /* config/i386/i386-modes.def:84 */
  V16DFmode,               /* config/i386/i386-modes.def:84 */
  V8TFmode,                /* config/i386/i386-modes.def:84 */
  MAX_MACHINE_MODE,

  MIN_MODE_RANDOM = VOIDmode,
  MAX_MODE_RANDOM = BLKmode,

  MIN_MODE_CC = CCmode,
  MAX_MODE_CC = CCFPUmode,

  MIN_MODE_INT = QImode,
  MAX_MODE_INT = XImode,

  MIN_MODE_PARTIAL_INT = VOIDmode,
  MAX_MODE_PARTIAL_INT = VOIDmode,

  MIN_MODE_FRACT = QQmode,
  MAX_MODE_FRACT = TQmode,

  MIN_MODE_UFRACT = UQQmode,
  MAX_MODE_UFRACT = UTQmode,

  MIN_MODE_ACCUM = HAmode,
  MAX_MODE_ACCUM = TAmode,

  MIN_MODE_UACCUM = UHAmode,
  MAX_MODE_UACCUM = UTAmode,

  MIN_MODE_FLOAT = SFmode,
  MAX_MODE_FLOAT = TFmode,

  MIN_MODE_DECIMAL_FLOAT = SDmode,
  MAX_MODE_DECIMAL_FLOAT = TDmode,

  MIN_MODE_COMPLEX_INT = CQImode,
  MAX_MODE_COMPLEX_INT = CXImode,

  MIN_MODE_COMPLEX_FLOAT = SCmode,
  MAX_MODE_COMPLEX_FLOAT = TCmode,

  MIN_MODE_VECTOR_INT = V2QImode,
  MAX_MODE_VECTOR_INT = V8TImode,

  MIN_MODE_VECTOR_FRACT = VOIDmode,
  MAX_MODE_VECTOR_FRACT = VOIDmode,

  MIN_MODE_VECTOR_UFRACT = VOIDmode,
  MAX_MODE_VECTOR_UFRACT = VOIDmode,

  MIN_MODE_VECTOR_ACCUM = VOIDmode,
  MAX_MODE_VECTOR_ACCUM = VOIDmode,

  MIN_MODE_VECTOR_UACCUM = VOIDmode,
  MAX_MODE_VECTOR_UACCUM = VOIDmode,

  MIN_MODE_VECTOR_FLOAT = V2SFmode,
  MAX_MODE_VECTOR_FLOAT = V8TFmode,

  NUM_MACHINE_MODES = MAX_MACHINE_MODE
};

#define CONST_MODE_SIZE
#define CONST_MODE_BASE_ALIGN
#define CONST_MODE_IBIT const
#define CONST_MODE_FBIT const

#define BITS_PER_UNIT (8)
#define MAX_BITSIZE_MODE_ANY_INT 128
#define MAX_BITSIZE_MODE_ANY_MODE (128*BITS_PER_UNIT)

#endif /* insn-modes.h */
