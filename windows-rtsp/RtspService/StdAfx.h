// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__38F6BFF3_C964_4C57_9FA5_8C69DB4ABCDA__INCLUDED_)
#define AFX_STDAFX_H__38F6BFF3_C964_4C57_9FA5_8C69DB4ABCDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>

typedef signed char         INT8, *PINT8;
typedef signed short        INT16, *PINT16;
typedef signed int          INT32, *PINT32;
typedef signed __int64      INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned __int64    UINT64, *PUINT64;

typedef unsigned char       uint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef signed short        int16_t;
typedef unsigned int        uint32_t;
typedef signed int          int32_t;

typedef unsigned __int64    uint64_t;
typedef signed __int64      int64_t;

typedef uint8_t             byte_t;

typedef int					bool_t;
typedef int64_t				mtime_t;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__38F6BFF3_C964_4C57_9FA5_8C69DB4ABCDA__INCLUDED_)
