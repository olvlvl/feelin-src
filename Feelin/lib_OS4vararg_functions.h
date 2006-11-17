#include <stdarg.h>

/*

VARARGS68K VOID SomeFuncTags (struct SomeLibIFace *Self, ...) 
{ 
    va_list args; 
    
    va_startlinear(args, Self); 
    Self->SomeFunc(Self, va_getlinearva(args,void *)); 
    va_end(args); 
}

*/


VARARGS68K void F_Alert(struct FeelinIFace *Self, STRPTR Title,STRPTR Body,...)
{
    va_list msg;

    va_startlinear(msg,Body);
    return Self -> F_AlertA(Title,Body, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K uint32 F_ClassDo(struct FeelinIFace *Self, FClass *Class,FObject Obj,uint32 Method,...)
{
    va_list msg;

    va_startlinear(msg,Method);
    return Self -> F_ClassDoA(Class,Obj,Method,va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K FClass * F_CreateClass(struct FeelinIFace *Self, STRPTR Name,...)
{
    va_list msg;

    va_startlinear(msg,Name);
    return Self -> F_CreateClassA(Name,va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K APTR F_CreatePool(struct FeelinIFace *Self, uint32 ItemSize, ...)
{
    va_list msg;

    va_startlinear(msg, ItemSize); 
    return Self -> F_CreatePoolA(ItemSize, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K uint32 F_Do(struct FeelinIFace *Self, FObject Obj,uint32 Method,...)
{
    va_list msg;

    va_startlinear(msg, Method);
    return Self -> F_DoA(Obj, Method, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K void F_Log(struct FeelinIFace *Self, uint32 Level,STRPTR Fmt,...)
{
    va_list msg;

    va_startlinear(msg, Fmt);
    return Self -> F_LogA(Level, Fmt, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K FObject F_MakeObj(struct FeelinIFace *Self, uint32 Type,...)
{
    va_list msg;

    va_startlinear(msg,Type);
    return Self -> F_MakeObjA(Type, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K FObject F_NewObj(struct FeelinIFace *Self, STRPTR Name,...)
{
    va_list msg;

    va_startlinear(msg,Name);
    return Self -> F_NewObjA(Name, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K STRPTR F_StrFmt(struct FeelinIFace *Self, APTR Buf, STRPTR Str,...)
{
    va_list msg;

    va_startlinear(msg,Str);
    return Self -> F_StrFmtA(Buf,Str, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K STRPTR F_StrNew(struct FeelinIFace *Self, uint32 *Length,STRPTR Fmt,...)
{
    va_list msg;

    va_startlinear(msg,Fmt);
    return Self -> F_StrNewA(Length, Fmt, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K STRPTR F_StrNewP(struct FeelinIFace *Self, APTR Pool,uint32 *Len,STRPTR Fmt,...)
{
    va_list msg;

    va_startlinear(msg,Fmt);
    return Self -> F_StrNewPA(Pool, Len, Fmt, va_getlinearva(msg, void *));
    va_end(msg);
}

VARARGS68K uint32 F_SuperDo(struct FeelinIFace *Self, FClass *Class,FObject Obj,uint32 Method,...)
{
    va_list msg;

    va_startlinear(msg,Method);
    return Self -> F_SuperDoA(Class, Obj, Method, va_getlinearva(msg, void *));
    va_end(msg);
}
