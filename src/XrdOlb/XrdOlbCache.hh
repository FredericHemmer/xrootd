#ifndef __OLB_CACHE__H
#define __OLB_CACHE__H
/******************************************************************************/
/*                                                                            */
/*                        X r d O l b C a c h e . h h                         */
/*                                                                            */
/* (c) 2003 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*                            All Rights Reserved                             */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*              DE-AC03-76-SFO0515 with the Department of Energy              */
/******************************************************************************/

//         $Id$
  
#include "XrdOlb/XrdOlbPList.hh"
#include "XrdOlb/XrdOlbScheduler.hh"
#include "XrdOlb/XrdOlbTypes.hh"
#include "XrdOuc/XrdOucHash.hh"
#include "XrdOuc/XrdOucPthread.hh"
 
/******************************************************************************/
/*                     S t r u c t   o o l b _ C I n f o                      */
/******************************************************************************/
  
struct XrdOlbCInfo
       {SMask_t rovec;
        SMask_t rwvec;
        int deadline;
       };

/******************************************************************************/
/*                      C l a s s   o o l b _ C a c h e                       */
/******************************************************************************/
  
class XrdOlbCache
{
public:
friend class XrdOlbCache_Scrubber;

XrdOlbPList_Anchor Paths;

void       AddFile(char *path, SMask_t mask, int isrw=-1, int dltime=0);
void       DelFile(char *path, SMask_t mask);
int        GetFile(char *path, XrdOlbCInfo &cinfo);

void       Apply(int (*func)(const char *, XrdOlbCInfo *, void *), void *Arg);

void       Extract(char *pathpfx, XrdOucHash<char> *hashp);

void       Reset(int servid);

void       Scrub();

void       setLifetime(int lsec) {LifeTime = lsec;}

           XrdOlbCache() {LifeTime = 8*60*60;}
          ~XrdOlbCache() {}   // Never gets deleted

private:

XrdOucMutex            PTMutex;
XrdOucHash<XrdOlbCInfo> PTable;
int                   LifeTime;
};
 
/******************************************************************************/
/*             C l a s s   o o l b _ C a c h e _ S c r u b b e r              */
/******************************************************************************/
  
class XrdOlbCache_Scrubber : public XrdOlbJob
{
public:

int   DoIt() {CacheP->Scrub(); 
              SchedP->Schedule((XrdOlbJob *)this, CacheP->LifeTime+time(0));
              return 1;
             }
      XrdOlbCache_Scrubber(XrdOlbCache *cp, XrdOlbScheduler *sp)
                        : XrdOlbJob("File cache scrubber")
                {CacheP = cp; SchedP = sp;}
     ~XrdOlbCache_Scrubber() {}

private:

XrdOlbScheduler *SchedP;
XrdOlbCache     *CacheP;
};
#endif
