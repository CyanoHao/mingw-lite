int __stdcall StdcallThunk2(int _1, int _2) { return 2; }

#ifdef _WIN64
typeof(&StdcallThunk2)
    dllimport_StdcallThunk2 asm("__imp_StdcallThunk2") = &StdcallThunk2;
#else
typeof(&StdcallThunk2)
    dllimport_StdcallThunk2 asm("__imp__StdcallThunk2@8") = &StdcallThunk2;
#endif
