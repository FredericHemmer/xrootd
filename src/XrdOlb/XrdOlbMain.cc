/******************************************************************************/
/*                                                                            */
/*                         X r d O l b M a i n . c c                          */
/*                                                                            */
/* (c) 2003 by the Board of Trustees of the Leland Stanford, Jr., University  */
/*                            All Rights Reserved                             */
/*   Produced by Andrew Hanushevsky for Stanford University under contract    */
/*              DE-AC03-76-SFO0515 with the Department of Energy              */
/******************************************************************************/

//         $Id$

const char *XrdOlbMainCVSID = "$Id$";
  
/* This is the distributed cache server. It can work in master scheduling
   mode (the -m option) or in slave mode (-s) option.

   oolbd [options] [configfn]

   options: [-d] [-l <fname>] [-L <sec>] [-m] [-s] [-w]

Where:
   -d     Turns on debugging mode.

   -l     Specifies location of the log file. By default, error messages
          go to standard out.

   -L     How many minutes between log file closes.

   -m     function in master scheduling moede.

   -s     Executes in slave mode.

   -w     Wait for a data-point connection before connecting to the master.

Notes:
   1.     The name of config file must either be specified on the command
          line or via the environmental variable XrdOlbCONFIGFN.
*/

/******************************************************************************/
/*                         i n c l u d e   f i l e s                          */
/******************************************************************************/
  
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <iostream.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <sys/param.h>

#include "Experiment/Experiment.hh"
#include "XrdOlb/XrdOlbAdmin.hh"
#include "XrdOlb/XrdOlbCache.hh"
#include "XrdOlb/XrdOlbConfig.hh"
#include "XrdOlb/XrdOlbScheduler.hh"
#include "XrdOlb/XrdOlbManager.hh"
#include "XrdOlb/XrdOlbPrepare.hh"
#include "XrdOlb/XrdOlbTrace.hh"
#include "XrdOuc/XrdOucError.hh"
#include "XrdOuc/XrdOucLink.hh"
#include "XrdOuc/XrdOucLogger.hh"
#include "XrdOuc/XrdOucNetwork.hh"
  
/******************************************************************************/
/*                        G l o b a l   O b j e c t s                         */
/******************************************************************************/

       int              XrdOlbSTDERR;

       XrdOlbCache      XrdOlbCache;

       XrdOlbConfig     XrdOlbConfig;

       XrdOlbPrepare    XrdOlbPrepQ;

       XrdOlbScheduler *XrdOlbSchedM  = 0;
       XrdOlbScheduler *XrdOlbSchedS  = 0;

       XrdOlbManager    XrdOlbSM;

       XrdOlbServer    *XrdOlbMasters = 0;

       XrdOucNetwork   *XrdOlbNetTCP  = 0;
       XrdOucNetwork   *XrdOlbNetUDPm = 0;
       XrdOucNetwork   *XrdOlbNetUDPs = 0;

       XrdOucLink      *XrdOlbRelay = 0;

       XrdOucLogger     XrdOlbLog;

       XrdOucError      XrdOlbSay(&XrdOlbLog, "olb_");

       XrdOucTrace      XrdOlbTrace(&XrdOlbSay);

/******************************************************************************/
/*            E x t e r n a l   T h r e a d   I n t e r f a c e s             */
/******************************************************************************/
  
extern "C"
{
void *XrdOlbLoginServer(void *carg)
      {XrdOucLink *lp = (XrdOucLink *)carg;
       return XrdOlbSM.Login(lp);
      }

void *XrdOlbStartAdmin(void *carg)
      {XrdOlbAdmin Admin;
       return Admin.Start((XrdOucSocket *)carg);
      }

void *XrdOlbStartAnote(void *carg)
      {XrdOlbAdmin Anote;
       return Anote.Notes((XrdOucSocket *)carg);
      }

void *XrdOlbStartPandering(void *carg)
      {XrdOucTList *tp = (XrdOucTList *)carg;
       return XrdOlbSM.Pander(tp->text, tp->val);
      }

void *XrdOlbStartUDP(void *carg)
      {int formaster = (int)carg;
       return XrdOlbSM.StartUDP(formaster);
      }
}

/******************************************************************************/
/*                                  m a i n                                   */
/******************************************************************************/
  
main(int argc, char *argv[])
{
   const char *epname = "main";
   XrdOucSemaphore SyncUp(0);
   pthread_t    tid;
   XrdOucLink   *newlink;
   XrdOucTList  *tp;

// Turn off sigpipe before we start any threads
//
   sigignore(SIGPIPE);
   sighold(SIGCHLD);

// Process configuration file
//
   if (XrdOlbConfig.Configure(argc, argv)) exit(1);

// Start the notification thread if we need to
//
   if (XrdOlbConfig.AnoteSock)
      {XrdOucThread_Run(&tid, XrdOlbStartAnote, (void *)XrdOlbConfig.AnoteSock);
       DEBUG("Main: Thread " <<tid <<" handling notification traffic.");
      }

// Start the admin thread if we need to, we will not continue until told
// to do so by the admin interface.
//
   if (XrdOlbConfig.AdminSock)
      {XrdOlbAdmin::setSync(&SyncUp);
       XrdOucThread_Run(&tid, XrdOlbStartAdmin, (void *)XrdOlbConfig.AdminSock);
       DEBUG("Main: Thread " <<tid <<" handling admin traffic.");
       SyncUp.Wait();
      }

// Start appropriate subsystems for master/slave operation. At the moment we
// only support one or the other but we act like we support both.
//
   if (XrdOlbConfig.Slave())
      {tp = XrdOlbConfig.myMasters;
       while(tp)
            {if (!XrdOlbConfig.Master() && !tp->next) 
                XrdOlbSM.Pander(tp->text, tp->val);
                else {if (XrdOucThread_Run(&tid,XrdOlbStartPandering,(void *)tp))
                         {XrdOlbSay.Emsg("oolbd", errno, "starting server");
                          _exit(1);
                         }
                       DEBUG("Main: Thread " <<(unsigned int)tid <<" pandering to " <<tp->text);
                      }
             tp = tp->next;
            }
      }

// Do master processing now, simply loop looking for connections
//
   if (XrdOlbConfig.Master())
       XrdOucThread_Run(&tid, XrdOlbStartUDP, (void *)1);
       DEBUG("Main: Thread " <<tid <<" handling master UDP traffic.");
       while(1) if (newlink = XrdOlbNetTCP->Accept())
                   {DEBUG("oolbd: FD " <<newlink->FDnum() <<" connected to " <<newlink->Name());
                    XrdOucThread_Run(&tid, XrdOlbLoginServer, (void *)newlink);
                   }

// If we ever get here, just exit
//
   exit(0);
}
