#ifndef _XRDOSS_API_H
#define _XRDOSS_API_H
/******************************************************************************/
/*                                                                            */
/*                          X r d O s s A p i . h h                           */
/*                                                                            */
/* (c) 2003 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*                            All Rights Reserved                             */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*              DE-AC03-76-SFO0515 with the Department of Energy              */
/******************************************************************************/

//         $Id$

#include <sys/types.h>
#include <errno.h>
#include <iostream.h>

#include "XrdOss/XrdOss.hh"
#include "XrdOss/XrdOssCache.hh"
#include "XrdOss/XrdOssConfig.hh"
#include "XrdOss/XrdOssError.hh"
#include "XrdOuc/XrdOucError.hh"
#include "XrdOuc/XrdOucPList.hh"
#include "XrdOuc/XrdOucStream.hh"

/******************************************************************************/
/*                              o o s s _ D i r                               */
/******************************************************************************/

class XrdOssDir : public XrdOssDF
{
public:
int     Close();
int     Opendir(const char *);
int     Readdir(char *buff, int blen);

        // Constructor and destructor
        XrdOssDir(const char *tid) 
                 {lclfd = 0; pflags = 0; ateof = 0; trfd = 0; tident = tid;}
       ~XrdOssDir() {if (trfd) Close();}
private:
      DIR  *lclfd;
const char *tident;
      int   pflags;
      int   ateof;
      int   trfd;
};
  
/******************************************************************************/
/*                             o o s s _ F i l e                              */
/******************************************************************************/

class oocx_CXFile;
  
class XrdOssFile : public XrdOssDF
{
public:

int     Close();
int     Fstat(struct stat *);
int     Fsync();
int     Ftruncate(unsigned long long);
int     isCompressed(char *cxidp=0);
int     Open(const char *, int, mode_t, XrdOucEnv &);
int     Read(               off_t, size_t);
size_t  Read(       void *, off_t, size_t);
size_t  ReadRaw(    void *, off_t, size_t);
size_t  Write(const void *, off_t, size_t);

XrdOucPListAnchor RPList;   // The remote path list
 
        // Constructor and destructor
        XrdOssFile(const char *tid)
                  {cxobj = 0; rawio = 0; cxpgsz = 0; cxid[0] = '\0';
                   tident = tid;
                  }

       ~XrdOssFile() {if (fd >= 0) Close();}

private:
int     Open_ufs(const char *, int, int, int);

oocx_CXFile *cxobj;
const char  *tident;
int          rawio;
int          cxpgsz;
char         cxid[4];
};

/******************************************************************************/
/*                              o o s s _ S y s                               */
/******************************************************************************/
  
class XrdOssSys : public XrdOss
{
public:
int       Chmod(const char *, mode_t mode);
void     *CacheScan(void *carg);
int       Configure(const char *, XrdOucError &);
void      Config_Display(XrdOucError &);
int       Create(const char *, mode_t, XrdOucEnv &);
int       GenLocalPath(const char *, char *);
int       GenRemotePath(const char *, char *);
int       Init(XrdOucLogger *, const char *);
int       IsRemote(const char *path) {return RPList.Find(path) & XrdOssREMOTE;}
int       Mkdir(const char *, mode_t mode);
int       PathOpts(const char *path) {return (RPList.Find(path) | XeqFlags);}
int       Rename(const char *, const char *);
int       Stage(const char *, XrdOucEnv &);
void     *Stage_In(void *carg);
int       Stat(const char *, struct stat *, int resonly=0);
int       Unlink(const char *);
   
int       MSS_Opendir(char *);
int       MSS_Readdir(int, char *buff, int blen);
int       MSS_Closedir(int);
int       MSS_Create(char *path, mode_t, XrdOucEnv &);
int       MSS_Stat(char *, struct stat *);
int       MSS_Unlink(char *);
int       MSS_Rename(char *, char *);

char     *ConfigFN;       // -> Pointer to the config file name
int       Hard_FD_Limit;  //    Hard file descriptor limit
char     *LocalRoot;      // -> Path prefix for local  filename
int       LocalRootLen;   //    Corresponding length
char     *RemoteRoot;     // -> Path prefix for remote filename
int       RemoteRootLen;  //    Corresponding length
char     *StageCmd;       // -> Staging command to use
char     *MSSgwCmd;       // -> MSS Gateway command to use
int       MSSgwCmdLen;    //    Corresponding length
char     *MSSgwPath;      // -> Socket path to talk to gateway.
long long MaxDBsize;      //    Maximum database size
int       gwBacklog;      //    Socket connection backlog.
int       FDFence;        //    Smallest file FD number allowed
int       FDLimit;        //    Largest  file FD number allowed
int       XeqFlags;       //    General execution flags
int       Trace;          //    Trace flags
int       ConvertFN;      //    If 1 filenames need to be converted
char     *CompSuffix;     // -> Compressed file suffix or null for autodetect
int       CompSuflen;     //    Length of suffix
int       Configured;     //    0 at start 1 ever after

XrdOucPListAnchor RPList;    //    The remote path list
   
        XrdOssSys() {static char *syssfx[] = {XRDOSS_SFX_LIST, 0};
                    sfx = syssfx; Configured = 0; xfrtcount = 0;
                    fsdata=0; fsfirst=0; fslast=0; fscurr=0; fsgroups=0;
                    xsdata=0; xsfirst=0; xslast=0; xscurr=0; xsgroups=0;
                    pndbytes=0; stgbytes=0; totbytes=0; totreqs=0; badreqs=0;
                    CompSuffix = 0; CompSuflen = 0;
                    StageQ.pendList.setItem(0);
                    StageQ.fullList.setItem(0);
                    ConfigDefaults();
                   }
       ~XrdOssSys() {}

private:
// Cache management related data and methods
//
long long minalloc;          //    Minimum allocation
int       ovhalloc;          //    Allocation overage
int       fuzalloc;          //    Allocation fuzz
int       cscanint;          //    Seconds between cache scans
long      xfrspeed;          //    Average transfer speed (bytes/second)
int       xfrovhd;           //    Minimum seconds to get a file
int       xfrhold;           //    Second hold limit on failing requests
int       xfrthreads;        //    Number of threads for staging
int       xfrtcount;         //    Actual count of threads (used for dtr)
long long pndbytes;          //    Total bytes to be staged (pending)
long long stgbytes;          //    Total bytes being staged (active)
long long totbytes;          //    Total bytes were  staged (active+pending)
long      totreqs;           //    Total   successful requests
long      badreqs;           //    Total unsuccessful requests

friend class XrdOssCache_FSData;
friend class XrdOssCache_FS;
friend class XrdOssCache_Group;
friend class XrdOssCache_Lock;

XrdOssCache_FSData *fsdata;   // -> Filesystem data
XrdOssCache_FS     *fsfirst;  // -> First  filesystem
XrdOssCache_FS     *fslast;   // -> Last   filesystem
XrdOssCache_FS     *fscurr;   // -> Curent filesystem (global allocation only)
XrdOssCache_Group  *fsgroups; // -> Cache group list

XrdOssCache_FSData *xsdata;   // -> Filesystem data   (config time only)
XrdOssCache_FS     *xsfirst;  // -> First  filesystem (config time only)
XrdOssCache_FS     *xslast;   // -> Last   filesystem (config time only)
XrdOssCache_FS     *xscurr;   // -> Curent filesystem (config time only)
XrdOssCache_Group  *xsgroups; // -> Cache group list  (config time only)

XrdOssCache_Req StageQ;       //    Queue of staging requests

XrdOucMutex     CacheContext;
XrdOucSemaphore ReadyRequest;

char **sfx;                  // -> Valid filename suffixes

int                Alloc_Cache(const char *path, mode_t amode, XrdOucEnv &env);
int                Alloc_Local(const char *path, mode_t amode, XrdOucEnv &env);
int                BreakLink(const char *local_path, struct stat &statbuff);
int                CalcTime(XrdOssCache_Req *req);
int                Find(XrdOssCache_Req *req, void *carg);
int                GetFile(XrdOssCache_Req *req);
int                HasFile(const char *fn, const char *sfx);
void               List_Cache(char *lname, int self, XrdOucError &Eroute);
void               ReCache();

// Configuration related methods
//
off_t  Adjust(dev_t devid, off_t size);
int    concat_fn(const char *, const int, const char *, char *);
void   ConfigDefaults(void);
int    ConfigProc(XrdOucError &Eroute);
int    ConfigXeq(char *, XrdOucStream &, XrdOucError &);
void   List_Flist(char *, XrdOucPListAnchor &, XrdOucError &);
int    xalloc(XrdOucStream &Config, XrdOucError &Eroute);
int    xcache(XrdOucStream &Config, XrdOucError &Eroute);
int    xcacheBuild(char *grp, char *fn, XrdOucError &Eroute);
int    xcompdct(XrdOucStream &Config, XrdOucError &Eroute);
int    xcachescan(XrdOucStream &Config, XrdOucError &Eroute);
int    xfdlimit(XrdOucStream &Config, XrdOucError &Eroute);
int    xgwbklg(XrdOucStream &Config, XrdOucError &Eroute);
int    xmaxdbsz(XrdOucStream &Config, XrdOucError &Eroute);
int    xpath(XrdOucStream &Config, XrdOucError &Eroute, int rpval=0);
int    xtrace(XrdOucStream &Config, XrdOucError &Eroute);
int    xxfr(XrdOucStream &Config, XrdOucError &Eroute);

// Mass storage related methods
//
void   MSS_Gateway(void);
int    MSS_Init(int);
int    tranmode(char *);
int    MSS_Xeq(char *, XrdOucStream **, int);

// Other methods
//
int    RenameLink(char *old_path, char *new_path);

XrdOucPListAnchor Config_RPList; // Temp area for actual RPList setup
};

/******************************************************************************/
/*                  A P I   S p e c i f i c   D e f i n e s                   */
/******************************************************************************/
  
// The following defines are used to translate symbolicly linked paths
//
#define XrdOssTPC '%'
#define XrdOssTAMP(dst, src) \
   while(*src) {*dst = (*src == '/' ? XrdOssTPC : *src); src++; dst++;}; *dst='\0'
#define XrdOssTRNP(src) \
   while(*src) {if (*src == '/') *src = XrdOssTPC; src++;}

// The Check_RO macro is valid only for XrdOssSys objects.
//
#define Check_RO(act, flags, path, opname) \
   XrdOssREMOTE & (flags = PathOpts(path)); \
   if ((XeqFlags & XrdOssNOTRW) || (flags & XrdOssNOTRW)) \
      return OssEroute.Emsg(#act, -XRDOSS_E8005, opname, path)
#endif
