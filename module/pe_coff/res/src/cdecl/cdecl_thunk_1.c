int cdecl_thunk_1() { return 1; }

#ifdef _WIN64
typeof(&cdecl_thunk_1)
    dllimport_cdecl_thunk_1 asm("__imp_cdecl_thunk_1") = &cdecl_thunk_1;
#else
typeof(&cdecl_thunk_1)
    dllimport_cdecl_thunk_1 asm("__imp__cdecl_thunk_1") = &cdecl_thunk_1;
#endif
