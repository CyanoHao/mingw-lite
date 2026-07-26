int cdecl_thunk_3() { return 3; }

#ifdef _WIN64
typeof(&cdecl_thunk_3)
    dllimport_cdecl_thunk_3 asm("__imp_cdecl_thunk_3") = &cdecl_thunk_3;
#else
typeof(&cdecl_thunk_3)
    dllimport_cdecl_thunk_3 asm("__imp__cdecl_thunk_3") = &cdecl_thunk_3;
#endif
