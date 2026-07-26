int __stdcall StdcallThunk3(int _1, int _2, int _3) { return 3; }

#ifdef _WIN64
typeof(&StdcallThunk3)
    dllimport_StdcallThunk3 asm("__imp_StdcallThunk3") = &StdcallThunk3;
#else
typeof(&StdcallThunk3)
    dllimport_StdcallThunk3 asm("__imp__StdcallThunk3@12") = &StdcallThunk3;
#endif
