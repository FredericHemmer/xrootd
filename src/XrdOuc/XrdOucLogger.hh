#ifndef __OOUC_LOGGER_H__
#define __OOUC_LOGGER_H__
/******************************************************************************/
/*                                                                            */
/*                         o o u c _ L o g g e r . h                          */
/*                                                                            */
/*(C) 2003 by the Board of Trustees of the Leland Stanford, Jr., University   */
/*      All Rights Reserved. See XrdVersion.cc for complete License Terms     */
/*Produced by Andrew Hanushevsky for Stanford University under contract       */
/*           DE-AC03-76-SFO0515 with the Deprtment of Energy                  */
/******************************************************************************/

//        $Id$

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

#include "XrdOuc/XrdOucPthread.hh"

class XrdOucLogger
{
public:
         XrdOucLogger(int ErrFD=STDERR_FILENO);

        ~XrdOucLogger() {if (ePath) free(ePath);}

// Bind allows you to bind standard error to a file with an optional periodic
// closing and opening of the file.
//
int Bind(const char *path, int intsec=0);

// Flush any pending output
//
void Flush() {fsync(eFD);}

// originalFD() returns the base FD that we started with
//
int  originalFD() {return baseFD;}

// Output data and optionally prefix with date/time
//
void Put(int iovcnt, struct iovec *iov);

// Return formated date/time (tbuff must be atleast 24 characters)
//
int Time(char *tbuff);

// traceBeg() obtains  the logger lock and returns a message header.
// traceEnd() releases the logger lock and returns a newline
//
char *traceBeg() {Logger_Mutex.Lock(); Time(TBuff); return TBuff;}
char  traceEnd() {Logger_Mutex.UnLock(); return '\n';}

private:

XrdOucMutex Logger_Mutex;
char       TBuff[24];        // Trace header buffer
int        eFD;
int        baseFD;
char      *ePath;
char       Filesfx[8];
time_t     eNTC;
int        eInt;
time_t     eNow;

int    ReBind(int dorename=1);
};
#endif
