int __stdcall StdcallThunk1(int _1) { return 1; }

#ifdef _WIN64
typeof(&StdcallThunk1)
    dllimport_StdcallThunk1 asm("__imp_StdcallThunk1") = &StdcallThunk1;
#else
typeof(&StdcallThunk1)
    dllimport_StdcallThunk1 asm("__imp__StdcallThunk1@4") = &StdcallThunk1;
#endif
