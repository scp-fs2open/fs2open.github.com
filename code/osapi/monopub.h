/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    monopub.h

Abstract:

    This module contains the PUBLIC (viewable by driver & Win32 apps)
    definitions for the IOCTLs supported by the MONO device driver.

Environment:

    Kernel & User mode

Revision History:

    03-22-93 : created

--*/



//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//

#define FILE_DEVICE_MONO  0x00008100



//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define MONO_IOCTL_INDEX  0x810



//
// The MONO device driver IOCTLs
//

#define IOCTL_MONO_PRINT          CTL_CODE(FILE_DEVICE_MONO,     \
                                           MONO_IOCTL_INDEX,     \
                                           METHOD_BUFFERED,      \
                                           FILE_ANY_ACCESS)

#define IOCTL_MONO_CLEAR_SCREEN   CTL_CODE(FILE_DEVICE_MONO,     \
                                           MONO_IOCTL_INDEX + 1, \
                                           METHOD_BUFFERED,      \
                                           FILE_ANY_ACCESS)
