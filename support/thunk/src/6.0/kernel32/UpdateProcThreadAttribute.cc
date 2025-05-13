#include <thunk/_common.h>
#include <thunk/yy/api-ms-win-core-processthreads.h>

#include <processthreadsapi.h>
#include <windows.h>

namespace mingw_thunk
{
  // YY-Thunks 1.1.6
  // Minimum supported client	Windows Vista [desktop apps only]
  // Minimum supported server	Windows Server 2008 [desktop apps only]
  __DEFINE_THUNK(kernel32,
                 28,
                 BOOL,
                 WINAPI,
                 UpdateProcThreadAttribute,
                 _Inout_ LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
                 _In_ DWORD dwFlags,
                 _In_ DWORD_PTR Attribute,
                 _In_reads_bytes_opt_(cbSize) PVOID lpValue,
                 _In_ SIZE_T cbSize,
                 _Out_writes_bytes_opt_(cbSize) PVOID lpPreviousValue,
                 _In_opt_ PSIZE_T lpReturnSize)
  {
    if (const auto pUpdateProcThreadAttribute =
            try_get_UpdateProcThreadAttribute()) {
      return pUpdateProcThreadAttribute(lpAttributeList,
                                        dwFlags,
                                        Attribute,
                                        lpValue,
                                        cbSize,
                                        lpPreviousValue,
                                        lpReturnSize);
    }

    LSTATUS lStatus = ERROR_SUCCESS;

    do {
      auto AttributeMark = 1ul << Attribute;

      /////////////////////////////////////////////////////
      //
      // 参数检查
      //

      if (dwFlags & (~PROC_THREAD_ATTRIBUTE_REPLACE_VALUE)) {
        lStatus = ERROR_INVALID_PARAMETER;
        break;
      }

      if ((Attribute & PROC_THREAD_ATTRIBUTE_ADDITIVE) == 0) {
        if (lpAttributeList->Count == lpAttributeList->Size) {
          // internal::BaseSetLastNTError(0xC0000001);
          lStatus = ERROR_GEN_FAILURE;
          break;
        } else if (AttributeMark & lpAttributeList->dwFlags) {
          // internal::BaseSetLastNTError(0x40000000);
          lStatus = ERROR_OBJECT_NAME_EXISTS;
          break;
        } else if (lpPreviousValue) {
          // internal::BaseSetLastNTError(0xC00000F4);
          lStatus = ERROR_INVALID_PARAMETER;
          break;
        } else if (dwFlags & PROC_THREAD_ATTRIBUTE_REPLACE_VALUE) {
          // internal::BaseSetLastNTError(0xC00000F0);
          lStatus = ERROR_INVALID_PARAMETER;
          break;
        }
      }

      if ((PROC_THREAD_ATTRIBUTE_INPUT & Attribute) && lpReturnSize) {
        // internal::BaseSetLastNTError(0xC00000F5);
        lStatus = ERROR_INVALID_PARAMETER;
        break;
      }

      //
      //
      ////////////////////////////////////////////////////

      auto pAttribute = &lpAttributeList->Entries[lpAttributeList->Count];

      constexpr auto ProcThreadAttributeExtendedFlags = 1;
      // 0x60001，文档没有公开
      constexpr auto PROC_THREAD_ATTRIBUTE_EXTENDED_FLAGS =
          ProcThreadAttributeValue(
              ProcThreadAttributeExtendedFlags, FALSE, TRUE, TRUE);

      if (Attribute == PROC_THREAD_ATTRIBUTE_PARENT_PROCESS) // 0x20000
      {
        // WinXP不支持 UAC，没实现似乎也没什么的。

        if (cbSize != sizeof(HANDLE)) {
          // internal::BaseSetLastNTError(0xC0000004);
          lStatus = ERROR_INVALID_PARAMETER;
          break;
        }
      } else if (Attribute == PROC_THREAD_ATTRIBUTE_EXTENDED_FLAGS) // 0x60001
      {
        // 系统没有公开这个含义，暂时让他允许通过把……

        if (cbSize != sizeof(DWORD)) {
          // internal::BaseSetLastNTError(0xC0000004);
          lStatus = ERROR_INVALID_PARAMETER;
          break;
        }

        DWORD dwOrgFlags;

        if (lpAttributeList->lpExtendedFlags) {
          pAttribute = lpAttributeList->lpExtendedFlags;
          dwOrgFlags = (DWORD)lpAttributeList->lpExtendedFlags->lpValue;
          AttributeMark = 0;
        } else {
          lpAttributeList->lpExtendedFlags = pAttribute;
          dwOrgFlags = 0;
        }

        auto dwNewFlags = *(DWORD *)lpValue;

        if (dwNewFlags & ~0x00000003ul) {
          // internal::BaseSetLastNTError(0xC000000D);
          lStatus = ERROR_BAD_LENGTH;
          break;
        }

        if ((dwFlags & PROC_THREAD_ATTRIBUTE_REPLACE_VALUE) == 0 &&
            dwOrgFlags) {
          dwNewFlags |= dwOrgFlags;
        }

        if (lpPreviousValue)
          *(DWORD *)lpPreviousValue = dwOrgFlags;

        lpValue = (PVOID)dwNewFlags;
      } else if (Attribute == PROC_THREAD_ATTRIBUTE_HANDLE_LIST) // 0x20002
      {
        // WinXP也不支持指定句柄继承，他会直接继承所有可继承的句柄，所以没实现好像也没什么大不了的。

        if (cbSize == 0 || cbSize % sizeof(HANDLE) != 0) {
          // internal::BaseSetLastNTError(0xC0000004);
          lStatus = ERROR_INVALID_PARAMETER;
          break;
        }
      } else {
        // internal::BaseSetLastNTError(0xC00000BB);
        lStatus = ERROR_NOT_SUPPORTED;
        break;
      }

      // LABEL_17
      pAttribute->lpValue = lpValue;

      if (AttributeMark) {
        pAttribute->Attribute = Attribute;
        pAttribute->cbSize = cbSize;
        ++lpAttributeList->Count;
        lpAttributeList->dwFlags |= AttributeMark;
      }

      return TRUE;

    } while (false);

    SetLastError(lStatus);

    return FALSE;
  }
} // namespace mingw_thunk
