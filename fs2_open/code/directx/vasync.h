/*==========================================================================;
 *
 *  Copyright (C) 1996-1997 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       async.h
 *  Content:    AsyncData object include file
 ***************************************************************************/
#ifndef __ASYNC_INCLUDED__
#define __ASYNC_INCLUDED__


#include <ole2.h>       // for DECLARE_INTERFACE and HRESULT

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * GUIDS used by DPAsyncData objects
 */

// {47BCD7E0-2E89-11d0-A889-00A0C905433C}
DEFINE_GUID(IID_IDPAsyncData, 0x47bcd7e0, 0x2e89, 0x11d0, 0xa8, 0x89, 0x0, 0xa0, 0xc9, 0x5, 0x43, 0x3c);


/****************************************************************************
 *
 * IDPAsyncData Structures
 *
 * Various structures used to invoke DPAsyncData.
 *
 ****************************************************************************/

typedef struct IDPAsyncData     FAR *LPDPASYNCDATA;


/****************************************************************************
 *
 * IDPAsyncData Interface
 *
 ****************************************************************************/

#undef INTERFACE
#define INTERFACE IDPAsyncData
DECLARE_INTERFACE_( IDPAsyncData, IUnknown )
{
    /*  IUnknown Methods	*/
    STDMETHOD(QueryInterface)       (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /*  IDPAsyncData Methods	*/
    STDMETHOD(AddItem)              (THIS_ LPDWORD, LPVOID, DWORD) PURE;
    STDMETHOD(Cancel)               (THIS) PURE;
    STDMETHOD(GetFlags)             (THIS_ LPDWORD) PURE;
    STDMETHOD(GetItem)              (THIS_ DWORD, LPVOID *) PURE;
    STDMETHOD(GetItemCount)         (THIS_ LPDWORD) PURE;
    STDMETHOD(GetStatus)            (THIS_ LPDWORD, LPDWORD, HRESULT *) PURE;
    STDMETHOD(IsSnapshotCurrent)    (THIS) PURE;
    STDMETHOD(RefreshSnapshot)      (THIS) PURE;
    STDMETHOD(RemoveItem)           (THIS_ DWORD) PURE;
    STDMETHOD(SetFlags)             (THIS_ DWORD) PURE;
    STDMETHOD(SetItem)              (THIS_ DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetStatus)            (THIS_ DWORD) PURE;
    STDMETHOD(SetStatusEvent)       (THIS_ DWORD, HANDLE) PURE;
};


/****************************************************************************
 *
 * IDPAsyncData interface macros
 *
 ****************************************************************************/

#if !defined(__cplusplus) || defined(CINTERFACE)

#define IDPAsyncData_QueryInterface(p,a,b)                  (p)->lpVtbl->QueryInterface(p,a,b)
#define IDPAsyncData_AddRef(p)                              (p)->lpVtbl->AddRef(p)
#define IDPAsyncData_Release(p)                             (p)->lpVtbl->Release(p)
#define IDPAsyncData_AddItem(p)                             (p)->lpVtbl->AddItem(p)
#define IDPAsyncData_Cancel(p)                              (p)->lpVtbl->Cancel(p)
#define IDPAsyncData_GetItem(p,a,b)                         (p)->lpVtbl->GetItem(p,a,b)
#define IDPAsyncData_GetItemCount(p,a)                      (p)->lpVtbl->GetItemCount(p,a)
#define IDPAsyncData_GetStatus(p,a,b,c)                     (p)->lpVtbl->GetStatus(p,a,b,c)
#define IDPAsyncData_IsSnapshotCurrent(p)                   (p)->lpVtbl->IsSnapshotCurrent(p)
#define IDPAsyncData_RefreshSnapshot(p)                     (p)->lpVtbl->RefreshSnapshot(p)
#define IDPAsyncData_RemoveItem(p)                          (p)->lpVtbl->RemoveItem(p)
#define IDPAsyncData_SetFlags(p)                            (p)->lpVtbl->SetFlags(p)
#define IDPAsyncData_SetItem(p)                             (p)->lpVtbl->SetItem(p)
#define IDPAsyncData_SetStatus(p)                           (p)->lpVtbl->SetStatus(p)
#define IDPAsyncData_SetStatusEvent(p,a,b)                  (p)->lpVtbl->SetStatusEvent(p,a,b)

#else /* C++ */

#define IDPAsyncData_QueryInterface(p,a,b)                  (p)->QueryInterface(a,b)
#define IDPAsyncData_AddRef(p)                              (p)->AddRef()
#define IDPAsyncData_Release(p)                             (p)->Release()
#define IDPAsyncData_AddItem(p)                             (p)->AddItem()
#define IDPAsyncData_Cancel(p)                              (p)->Cancel()
#define IDPAsyncData_GetItem(p,a,b)                         (p)->GetItem(a,b)
#define IDPAsyncData_GetItemCount(p,a)                      (p)->GetItemCount(a)
#define IDPAsyncData_GetStatus(p,a,b,c)                     (p)->GetStatus(a,b,c)
#define IDPAsyncData_IsSnapshotCurrent(p)                   (p)->IsSnapshotCurrent()
#define IDPAsyncData_RefreshSnapshot(p)                     (p)->RefreshSnapshot()
#define IDPAsyncData_RemoveItem(p)                          (p)->RemoveItem()
#define IDPAsyncData_SetFlags(p)                            (p)->SetFlags()
#define IDPAsyncData_SetItem(p)                             (p)->SetItem()
#define IDPAsyncData_SetStatus(p)                           (p)->SetStatus()
#define IDPAsyncData_SetStatusEvent(p,a,b)                  (p)->SetStatusEvent(a,b)

#endif


/****************************************************************************
 *
 * AsyncData Flags
 *
 ****************************************************************************/

/*
 *	This flag is set if the ItemData is ANSI.  If it is not set, the ItemData
 *	is either Unicode or doesn't contain any string information.
 */
#define DPASYNCDATA_ANSI			(0x00000001)


#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif /* __ASYNC_INCLUDED__ */
