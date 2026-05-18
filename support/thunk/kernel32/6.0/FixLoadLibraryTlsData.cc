#include "FixLoadLibraryTlsData.h"

#include <thunk/_common.h>

#include <windows.h>
#include <winternl.h>

extern "C" ULONG _tls_index;
#ifdef _WIN64
extern "C" const IMAGE_TLS_DIRECTORY64 _tls_used;
#else
extern "C" const IMAGE_TLS_DIRECTORY _tls_used;
#endif

extern "C" __attribute__((dllimport)) void NTAPI RtlGetNtVersionNumbers(
  LPDWORD, LPDWORD, LPDWORD);

namespace mingw_thunk::tls_fix
{
  enum class TlsMode : int
  {
    None,
    ByFirstCallback,
    ByDllMain,
  };

  struct TlsNode
  {
    TlsNode *next;
    TlsNode *prev;
    BYTE *base;
    void **old_index;
  };

  static volatile long s_lock = 0;
  static TlsNode *s_list = nullptr;
  static TlsMode s_mode = TlsMode::None;
  static ULONG s_tls_index_old = 0;

#pragma section(".CRT$XLB", long, read)

  static void NTAPI first_callback(PVOID, DWORD reason, PVOID)
  {
    if (reason == DLL_PROCESS_ATTACH && s_mode == TlsMode::None)
      s_mode = TlsMode::ByFirstCallback;
  }

  __declspec(allocate(".CRT$XLB")) static const PIMAGE_TLS_CALLBACK
    s_first_cb = first_callback;

  static bool is_vista_plus()
  {
    static bool cached = [] {
      DWORD major = 0;
      RtlGetNtVersionNumbers(&major, nullptr, nullptr);
      return major >= 6;
    }();
    return cached;
  }

  static SIZE_T tls_raw_size()
  {
    auto begin = reinterpret_cast<ULONG_PTR>(_tls_used.StartAddressOfRawData);
    auto end = reinterpret_cast<ULONG_PTR>(_tls_used.EndAddressOfRawData);
    return end > begin ? end - begin : 0;
  }

  static void list_add(TlsNode *node)
  {
    node->prev = nullptr;
    node->next = s_list;
    if (s_list)
      s_list->prev = node;
    s_list = node;
  }

  static void list_remove(TlsNode *node)
  {
    if (node->prev)
      node->prev->next = node->next;
    else
      s_list = node->next;
    if (node->next)
      node->next->prev = node->prev;
  }

  static TlsNode *list_flush()
  {
    return (TlsNode *)InterlockedExchangePointer((PVOID *)&s_list, nullptr);
  }

  static SIZE_T get_tls_index_count(TEB *teb)
  {
    auto ptr = (void **)teb->ThreadLocalStoragePointer;
    if (!ptr)
      return 0;
    return HeapSize(teb->ProcessEnvironmentBlock->ProcessHeap, 0, ptr) /
           sizeof(void *);
  }

  static PIMAGE_TLS_DIRECTORY find_tls_directory(HMODULE mod)
  {
    auto dos = (PIMAGE_DOS_HEADER)mod;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
      return nullptr;
    auto nt = (PIMAGE_NT_HEADERS)((BYTE *)mod + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE)
      return nullptr;
    auto &dir =
      nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    if (dir.VirtualAddress == 0 || dir.Size == 0)
      return nullptr;
    return (PIMAGE_TLS_DIRECTORY)((BYTE *)mod + dir.VirtualAddress);
  }

  static ULONG get_max_tls_index()
  {
    ULONG max_idx = 0;
    auto peb = NtCurrentTeb()->ProcessEnvironmentBlock;
    auto ldr = peb->Ldr;
    for (auto it = ldr->InMemoryOrderModuleList.Flink;
         it != &ldr->InMemoryOrderModuleList; it = it->Flink)
    {
      auto entry = CONTAINING_RECORD(it, LDR_DATA_TABLE_ENTRY,
                                     InMemoryOrderLinks);
      auto tls = find_tls_directory((HMODULE)entry->DllBase);
      if (!tls || !tls->AddressOfIndex)
        continue;
      __try
      {
        auto idx = *(ULONG *)tls->AddressOfIndex;
        if (idx > max_idx)
          max_idx = idx;
      }
      __except (EXCEPTION_EXECUTE_HANDLER)
      {
      }
    }
    return max_idx;
  }

  static bool alloc_tls_data(TEB *teb)
  {
    auto raw_size = tls_raw_size();
    if (raw_size == 0)
      return true;
    if (_tls_index == 0)
      return false;

    if (!teb)
      teb = NtCurrentTeb();

    auto heap = teb->ProcessEnvironmentBlock->ProcessHeap;
    auto tls_ptr = (void **)teb->ThreadLocalStoragePointer;
    auto count = get_tls_index_count(teb);

    auto buf =
      (BYTE *)HeapAlloc(heap, HEAP_ZERO_MEMORY, raw_size + sizeof(TlsNode));
    if (!buf)
      return false;

    auto node = (TlsNode *)(buf + raw_size);
    node->base = buf;
    node->old_index = nullptr;

    if (count <= _tls_index)
    {
      auto new_count = _tls_index + 128;
      auto new_ptr = (void **)HeapAlloc(heap, HEAP_ZERO_MEMORY,
                                         new_count * sizeof(void *));
      if (!new_ptr)
      {
        HeapFree(heap, 0, buf);
        return false;
      }
      memcpy(new_ptr, tls_ptr, count * sizeof(void *));
      if ((void *)InterlockedCompareExchangePointer(
            (PVOID *)&teb->ThreadLocalStoragePointer, (PVOID)new_ptr,
            (PVOID)tls_ptr) != tls_ptr)
      {
        HeapFree(heap, 0, new_ptr);
        HeapFree(heap, 0, buf);
        return false;
      }
      if (teb == NtCurrentTeb())
        HeapFree(heap, 0, tls_ptr);
      else
        node->old_index = tls_ptr;
      tls_ptr = new_ptr;
    }

    memcpy(buf, (void *)_tls_used.StartAddressOfRawData, raw_size);

    while (InterlockedCompareExchange(&s_lock, 1, 0) != 0)
      ;
    list_add(node);
    InterlockedExchange(&s_lock, 0);

    InterlockedExchangePointer((PVOID *)&tls_ptr[_tls_index], (PVOID)buf);
    return true;
  }

  static void free_tls_data()
  {
    if (_tls_index == 0)
      return;
    auto raw_size = tls_raw_size();
    if (raw_size == 0)
      return;

    auto teb = NtCurrentTeb();
    if (_tls_index >= get_tls_index_count(teb))
      return;

    auto tls_ptr = (void **)teb->ThreadLocalStoragePointer;
    auto data =
      (BYTE *)InterlockedExchangePointer(&tls_ptr[_tls_index], nullptr);
    if (!data)
      return;

    auto node = (TlsNode *)(data + raw_size);
    auto heap = teb->ProcessEnvironmentBlock->ProcessHeap;

    while (InterlockedCompareExchange(&s_lock, 1, 0) != 0)
      ;
    __try
    {
      if (node->base == data)
        list_remove(node);
      else
        node = nullptr;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
      node = nullptr;
    }
    InterlockedExchange(&s_lock, 0);

    if (node)
    {
      if (node->old_index)
        HeapFree(heap, 0, node->old_index);
      HeapFree(heap, 0, data);
    }
  }

  static bool create_tls_index()
  {
    if (_tls_index != 0)
      return false;

    _tls_index = get_max_tls_index() + 1;
    return alloc_tls_data(NtCurrentTeb());
  }

  static void free_tls_index()
  {
    if (_tls_index == 0)
      return;

    while (InterlockedCompareExchange(&s_lock, 1, 0) != 0)
      ;
    auto heap = NtCurrentTeb()->ProcessEnvironmentBlock->ProcessHeap;
    for (auto node = list_flush(); node;)
    {
      auto next = node->next;
      if (node->old_index)
        HeapFree(heap, 0, node->old_index);
      auto raw_size = tls_raw_size();
      HeapFree(heap, 0, node->base ? node->base : (BYTE *)node - raw_size);
      node = next;
    }
    InterlockedExchange(&s_lock, 0);
  }

  static void call_tls_callbacks(DWORD reason)
  {
    if (_tls_index == 0)
      return;
    auto teb = NtCurrentTeb();
    auto tls_ptr = (void **)teb->ThreadLocalStoragePointer;
    if (!tls_ptr)
      return;
    if (_tls_index >= get_tls_index_count(teb))
      return;
    if (!tls_ptr[_tls_index])
      return;

    __try
    {
      auto callbacks = reinterpret_cast<PIMAGE_TLS_CALLBACK *>(
        _tls_used.AddressOfCallBacks);
      if (callbacks)
      {
        for (; *callbacks; ++callbacks)
          (*callbacks)((HINSTANCE)&_tls_used, reason, nullptr);
      }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
  }

  void fix_pre(DWORD reason, LPVOID reserved)
  {
    (void)reserved;
    if (is_vista_plus())
      return;

    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
      if (s_mode == TlsMode::None)
        s_mode = TlsMode::ByDllMain;
      s_tls_index_old = _tls_index;
      if (_tls_index == 0 && s_mode == TlsMode::ByDllMain)
      {
        if (!create_tls_index())
          return;
      }
      break;

    case DLL_THREAD_ATTACH:
      if (s_tls_index_old == 0 && s_mode == TlsMode::ByDllMain)
        alloc_tls_data(nullptr);
      break;

    case DLL_THREAD_DETACH:
      if (s_tls_index_old == 0 && s_mode == TlsMode::ByDllMain)
        call_tls_callbacks(reason);
      break;

    case DLL_PROCESS_DETACH:
      if (s_tls_index_old == 0 && s_mode == TlsMode::ByDllMain)
        call_tls_callbacks(reason);
      break;
    }
  }

  void fix_post(DWORD reason, LPVOID reserved)
  {
    (void)reserved;
    if (is_vista_plus())
      return;

    switch (reason)
    {
    case DLL_THREAD_DETACH:
      if (s_tls_index_old == 0 && s_mode == TlsMode::ByDllMain)
        free_tls_data();
      break;

    case DLL_PROCESS_DETACH:
      if (s_tls_index_old == 0 && s_mode == TlsMode::ByDllMain)
        free_tls_index();
      break;
    }
  }
}

extern "C" void __cdecl __mingw_thunk_fix_load_library_tls_data(
  DWORD reason, LPVOID reserved, int is_pre)
{
  if (is_pre)
    mingw_thunk::tls_fix::fix_pre(reason, reserved);
  else
    mingw_thunk::tls_fix::fix_post(reason, reserved);
}
