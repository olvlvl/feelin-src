/*

$VER: 02.00 (2005/12/11)

    Thread design totaly rewritten from top to bottom !
    
$VER: 01.10 (2005/02/01)
 
    The priority of the process can now be set / get.  The  thread  process
    can be get with the FA_Thread_Process attribute.
 
$VER: 01.00 (2004/09/05)
   
    The FC_Thread class is basicaly an interface to the NewCreateProcess().

*/

#include "Project.h"

///METHODS
F_METHOD_PROTO(void, Thread_New);
F_METHOD_PROTO(void, Thread_Dispose);
F_METHOD_PROTO(void, Thread_Get);
F_METHOD_PROTO(void, Thread_Set);
F_METHOD_PROTO(void, Thread_Send);
F_METHOD_PROTO(void, Thread_Pop);
F_METHOD_PROTO(void, Thread_Wait);
//+

///QUERY
F_QUERY()
{
    switch (Which)
    {
        case FV_Query_ClassTags:
        {
            STATIC F_VALUES_ARRAY(Values_Priority) =
            {
                F_VALUES_ADD("Idle", -127),
                F_VALUES_ADD("Low", -10),
                F_VALUES_ADD("Normal", 0),
                F_VALUES_ADD("High", 10),
                
                F_ARRAY_END
            };

            STATIC F_ATTRIBUTES_ARRAY =
            {
                F_ATTRIBUTES_ADD("Entry", FV_TYPE_POINTER),
                F_ATTRIBUTES_ADD("StackSize", FV_TYPE_INTEGER),
                F_ATTRIBUTES_ADD_VALUES("Priority", FV_TYPE_INTEGER, Values_Priority),
                F_ATTRIBUTES_ADD("Name", FV_TYPE_STRING),
                F_ATTRIBUTES_ADD("Process", FV_TYPE_POINTER),
                F_ATTRIBUTES_ADD("UserData", FV_TYPE_POINTER),
                F_ATTRIBUTES_ADD("Port", FV_TYPE_POINTER),

                F_ARRAY_END
            };

            STATIC F_METHODS_ARRAY =
            {
                F_METHODS_ADD(Thread_Send, "Send"),
                F_METHODS_ADD(Thread_Pop, "Pop"),
                F_METHODS_ADD(Thread_Wait, "Wait"),

                F_METHODS_ADD_STATIC(Thread_New, FM_New),
                F_METHODS_ADD_STATIC(Thread_Dispose, FM_Dispose),
                F_METHODS_ADD_STATIC(Thread_Get, FM_Get),
                F_METHODS_ADD_STATIC(Thread_Set, FM_Set),

                F_ARRAY_END
            };

            STATIC F_TAGS_ARRAY =
            {
                F_TAGS_ADD_LOD,
                F_TAGS_ADD_METHODS,
                F_TAGS_ADD_ATTRIBUTES,

                F_ARRAY_END
            };

            return F_TAGS_PTR;
        }
    }
    return NULL;
}
//+

