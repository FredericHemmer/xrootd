#ifndef _XROOTD_FILE_H_
#define _XROOTD_FILE_H_
/******************************************************************************/
/*                                                                            */
/*                         x r o o t d _ F i l e . h                          */
/*                                                                            */
/* (c) 2003 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*      All Rights Reserved. See XrdVersion.cc for complete License Terms     */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*              DE-AC03-76-SFO0515 with the Department of Energy              */
/******************************************************************************/
  
//       $Id$

#include <string.h>

/******************************************************************************/
/*                              x r d _ F i l e                               */
/******************************************************************************/

class XrdSfsFile;
class XrdXrootdFileLock;

class XrdXrootdFile
{
public:

XrdSfsFile  *XrdSfsp;           // -> Actual file object
char         FileKey[38];       // -> Unique hash name for the file
char         FileMode;          // 'r' or 'w'
char         AsyncMode;         // 1 -> if file in async r/w mode
char        *ID;                // File user

static void Init(XrdXrootdFileLock *lp) {Locker = lp;}

           XrdXrootdFile(char *id, XrdSfsFile *fp, char mode='r', char async='\0');
          ~XrdXrootdFile();

private:
int bin2hex(char *outbuff, char *inbuff, int inlen);
static XrdXrootdFileLock *Locker;
static const char      *TraceID;
};
 
/******************************************************************************/
/*                      x r o o t d _ F i l e T a b l e                       */
/******************************************************************************/

// The before define the structure of the file table. We will have FTABSIZE
// internal table entries. We will then provide an external linear table
// that increases by FTABSIZE entries. There is one file table per link and
// it is owned by the base protocol object.
//
#define XRD_FTABSIZE   16
  
// WARNING! Manipulation (i.e., Add/Del/delete) of this object must be
//          externally serialized at the link level. Only one thread
//          may be active w.r.t this object during manipulation!
//
class XrdXrootdFileTable
{
public:

       long           Add(XrdXrootdFile *fp);

       void           Del(long fnum);

inline XrdXrootdFile *Get(long fnum)
                         {if (fnum < XRD_FTABSIZE) return FTab[fnum];
                          if (XTab && (fnum-XRD_FTABSIZE)<XTnum)
                             return XTab[fnum-XRD_FTABSIZE];
                          return (XrdXrootdFile *)0;
                         }

       XrdXrootdFileTable() {memset((void *)FTab, 0, sizeof(FTab));
                             FTfree = 0; XTab = 0; XTnum = XTfree = 0;
                            }
      ~XrdXrootdFileTable();

private:

static const char *TraceID;

XrdXrootdFile *FTab[XRD_FTABSIZE];
int            FTfree;

XrdXrootdFile **XTab;
int             XTnum;
int             XTfree;
};
#endif
