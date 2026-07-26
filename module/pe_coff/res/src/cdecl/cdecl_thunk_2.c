int cdecl_thunk_2() { return 2; }

#ifdef _WIN64
typeof(&cdecl_thunk_2)
    dllimport_cdecl_thunk_2 asm("__imp_cdecl_thunk_2") = &cdecl_thunk_2;
#else
typeof(&cdecl_thunk_2)
    dllimport_cdecl_thunk_2 asm("__imp__cdecl_thunk_2") = &cdecl_thunk_2;
#endif
