#include "Private.h"

//#define DB_NOTIFY
//#define DB_ADDNOTIFY
//#define DB_REMNOTIFY

///Thread_Main
F_THREAD_ENTRY(Thread_Main)
{
    struct MsgPort *port = IEXEC CreateMsgPort();
            
    uint8 read_again = FALSE;

    for (;;)
    {
        FThreadMsg *msg = (FThreadMsg *) IFEELIN F_Do(Thread, Public->id_Pop);

/*** read messages *****************************************************************************/

        if (msg)
        {
            read_again = TRUE;

            switch (msg->Action)
            {
                case FV_Thread_Hello:
                {
                    if (port)
                    {
                        msg->Return = TRUE;
                    }
                }
                break;

                case FV_Thread_Bye:
                {
                    
                    /* We get the 'bye' message, when there is not a  single
                    DOSNotify  object alive, thus, no notify request either.
                    To be safe, we flush the message port  and  destroy  it.
                    There shall be no notify request alive. */
 
                    if (port)
                    {
                        struct Message *pending;
                        
                        IEXEC Forbid();
                        
                        while ((pending = IEXEC GetMsg(port)) != NULL)
                        {
                            IEXEC ReplyMsg(pending);
                        }

                        IEXEC DeleteMsgPort(port);
                        
                        IEXEC Permit();
                    }
                    
                    msg->Return = TRUE;
 
                    return msg;
                }
                break;

                case FV_Thread_AddNotify:
                {
                    struct FS_Thread_AddNotify *par = (struct FS_Thread_AddNotify *) msg->Params;
                    struct NotifyRequest *nr = par->NReq;

                    if (nr && port)
                    {
                        #ifdef DB_ADDNOTIFY
                        IFEELIN F_Log(0,"Adding NR 0x%08lx - Name '%s'",nr,nr->nr_Name);
                        #endif

                        nr->nr_Flags = NRF_SEND_MESSAGE;
                        nr->nr_UserData = (uint32)(par->Object);
                        nr->nr_stuff.nr_Msg.nr_Port = port;

                        if (IDOS_ StartNotify(nr))
                        {
                            msg->Return = TRUE;
                        }
                    }
                }
                break;

                case FV_Thread_RemNotify:
                {
                    struct FS_Thread_RemNotify *par = (struct FS_Thread_RemNotify *) msg->Params;
                    struct NotifyRequest *nr = par->NReq;

                    if (nr)
                    {
                        #ifdef DB_REMNOTIFY
                        IFEELIN F_Log(0,"Removing NR 0x%08lx - Name '%s'",nr,nr->nr_Name);
                        #endif

                        IDOS_ EndNotify(nr);
                    }
                }
                break;
            }

            IEXEC ReplyMsg((struct Message *) msg);
        }
                
/*** dos notify port ***************************************************************************/
    
        if (port)
        {
            #ifdef F_ENABLE_CLONE_WORKAROUND
            
            struct NotifyRequest *last_nr = NULL;
            
            while ((msg = (FThreadMsg *) IEXEC GetMsg(port)) != NULL)
            {
                read_again = TRUE;
                
                if (last_nr == ((struct NotifyMessage *)(msg))->nm_NReq)
                {
                    #ifdef DB_NOTIFY

                    IFEELIN F_Log
                    (
                        0, "NOTIFY: request (0x%08lx) file (%s) - IGNORED !!",

                            ((struct NotifyMessage *)(msg))->nm_NReq,
                            ((struct NotifyMessage *)(msg))->nm_NReq->nr_Name
                    );

                    #endif
                    
                    IEXEC ReplyMsg((struct Message *) msg);
                }
                else
                {
                    
                    #ifdef DB_NOTIFY
                     
                    IFEELIN F_Log
                    (
                        0, "NOTIFY: request (0x%08lx) file (%s)",

                            ((struct NotifyMessage *)(msg))->nm_NReq,
                            ((struct NotifyMessage *)(msg))->nm_NReq->nr_Name
                    );
                    
                    #endif
                    
                    last_nr = ((struct NotifyMessage *)(msg))->nm_NReq;
                
                    IEXEC ReplyMsg((struct Message *) msg);
 
                    if (last_nr)
                    {
                        IFEELIN F_Set((FObject) last_nr->nr_UserData, CUD->id_Update, (uint32) last_nr->nr_Name);
                    }
                }
            }
            
            #else
            
            msg = (FThreadMsg *) IEXEC GetMsg(port);

            if (msg)
            {
                read_again = TRUE;

                #ifdef DB_NOTIFY
                IFEELIN F_Log
                (
                    0, "NOTIFY: request (0x%08lx) - flags (0x%08lx) - file (%s)",
                    
                        ((struct NotifyMessage *)(msg))->nm_NReq,
                        ((struct NotifyMessage *)(msg))->nm_NReq->nr_Flags,
                        ((struct NotifyMessage *)(msg))->nm_NReq->nr_Name
                );
                #endif

                IEXEC ReplyMsg((struct Message *) msg);

                IFEELIN F_Set((FObject)(((struct NotifyMessage *)(msg))->nm_NReq->nr_UserData),CUD->id_Update,(uint32)(((struct NotifyMessage *)(msg))->nm_NReq->nr_Name));
            }
        
            #endif
        }

/*** waiting ***********************************************************************************/
 
        if (read_again)
        {
            read_again = FALSE;
        }
        else
        {
            IFEELIN F_Do(Thread, Public->id_Wait, (port) ? (1 << port->mp_SigBit) : 0);
        }
    }
}
//+
