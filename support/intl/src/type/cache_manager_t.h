#pragma once

#include "../stl/basic_string.h"

#include "domain_cache.h"

namespace intl
{
  struct cache_manager_t
  {
  private:
    struct entry
    {
      string key;
      domain_cache value;

      entry() = default;
      entry(string &&key) : key(static_cast<string &&>(key))
      {
      }
      entry(string &&key, domain_cache &&value)
          : key(static_cast<string &&>(key))
          , value(static_cast<domain_cache &&>(value))
      {
      }
      entry(const entry &other) = delete;
      entry &operator=(const entry &other) = delete;
      entry(entry &&other) = default;
      entry &operator=(entry &&other) = default;
      ~entry() = default;

      void *operator new(size_t, void *ptr)
      {
        return ptr;
      }
    };

  private:
    entry *data_;
    size_t size_;
    size_t capacity_;

  public:
    cache_manager_t() : data_(nullptr), size_(0), capacity_(0)
    {
    }

    ~cache_manager_t()
    {
      for (size_t i = 0; i < size_; ++i)
        data_[i].~entry();
      free(data_);
    }

    cache_manager_t(const cache_manager_t &) = delete;
    cache_manager_t &operator=(const cache_manager_t &) = delete;

    cache_manager_t(cache_manager_t &&other)
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_)
    {
      other.data_ = nullptr;
      other.size_ = 0;
      other.capacity_ = 0;
    }

    cache_manager_t &operator=(cache_manager_t &&other)
    {
      // intentionally not check self move assignment
      for (size_t i = 0; i < size_; ++i)
        data_[i].~entry();
      free(data_);

      data_ = other.data_;
      size_ = other.size_;
      capacity_ = other.capacity_;

      other.data_ = nullptr;
      other.size_ = 0;
      other.capacity_ = 0;

      return *this;
    }

    domain_cache *get_or_null(const string &domain)
    {
      for (size_t i = 0; i < size_; ++i)
        if (data_[i].key == domain)
          return &data_[i].value;
      return nullptr;
    }

    domain_cache *allocate_slot(string &&domain)
    {
      if (size_ == capacity_) {
        size_t new_cap = capacity_ * 2;
        entry *new_data = (entry *)malloc(new_cap * sizeof(entry));
        if (!new_data)
          return nullptr;

        for (size_t i = 0; i < size_; ++i) {
          new (&new_data[i]) entry(static_cast<entry &&>(data_[i]));
        }

        data_ = new_data;
        capacity_ = new_cap;
      }

      new (&data_[size_]) entry(static_cast<string &&>(domain));
      return &data_[size_++].value;
    }

    domain_cache *get_or_push(const string &domain)
    {
      if (domain_cache *val = get_or_null(domain))
        return val;

      wstring prefix = resolve_prefix();
      if (prefix.empty())
        return nullptr;

      printf("prefix = %ls\n", prefix.c_str());

      // search order:
      // 1. "LANGUAGE"
      // 2. "LC_ALL"
      // 3. "LC_xxx"
      // 4. "LANG"
      // https://www.gnu.org/software/gettext/manual/html_node/Locale-Environment-Variables.html
      // undocumented: GNU gettext will try GetThreadLocale()
      // undocumented: set console to UTF-8 breaks GetThreadLocale()
      // extension: use GetUserPreferredUILanguages() instead
      const wchar_t *env_language = _wgetenv(L"LANGUAGE");
      const wchar_t *env_lc_all = _wgetenv(L"LC_ALL");
      const wchar_t *env_lc_messages = _wgetenv(L"LC_MESSAGES");
      const wchar_t *env_lang = _wgetenv(L"LANG");

      file fp;
      if (env_language != nullptr && *env_language != L'\0')
        fp = find_locale_file_by_env_language(prefix, env_language, domain);
      else if (env_lc_all != nullptr && *env_lc_all != L'\0')
        fp = find_locale_file(prefix, env_lc_all, domain);
      else if (env_lc_messages != nullptr && *env_lc_messages != L'\0')
        fp = find_locale_file(prefix, env_lc_messages, domain);
      else if (env_lang != nullptr && *env_lang != L'\0')
        fp = find_locale_file(prefix, env_lang, domain);
      else
        fp = find_locale_file_from_win32(prefix, domain);

      if (!fp)
        return nullptr;

      printf("found\n");

      string domain_copy = domain;
      domain_cache entry = domain_cache::from_file(static_cast<file &&>(fp));
      if (domain_cache *val =
              allocate_slot(static_cast<string &&>(domain_copy))) {
        *val = static_cast<domain_cache &&>(entry);
        return val;
      }
      return nullptr;
    }

  private:
    wstring resolve_prefix()
    {
      wstring path(MAX_PATH, 0);
      DWORD exe_len = GetModuleFileNameW(nullptr, &path[0], MAX_PATH);
      if (exe_len >= MAX_PATH)
        return {};

      // \lib\gcc\<target>\<version>\cc1.exe
      wstring_view target = LR"(\lib\gcc\)";
      size_t pos = path.rfind(target);
      if (pos == wstring::npos) {
        target = LR"(\bin\)";
        pos = path.rfind(target);
      }
      if (pos == wstring::npos)
        return {};

      path.resize(pos);
      path += LR"(\share\locale\)";
      return path;
    }

    file find_locale_file_by_env_language(wstring_view prefix,
                                          wstring_view env_language,
                                          string_view domain)
    {
      // LANGUAGE=zh_CN:en_US
      size_t b = 0;
      while (true) {
        size_t e = env_language.find(L':', b);
        wstring_view lang;
        if (e == wstring_view::npos)
          lang = env_language.substr(b);
        else
          lang = env_language.substr(b, e - b);
        file fp = find_locale_file(prefix, lang, domain);
        if (fp)
          return fp;
        if (e == wstring_view::npos)
          break;
        else
          b = e + 1;
      }
      return {};
    }

    file find_locale_file_by_win32_language_name(wstring_view prefix,
                                                 wstring_view language_name,
                                                 string_view domain)
    {
      // https://learn.microsoft.com/en-us/windows/win32/intl/language-names

      // remove supplemental: en-US-x-fabricam
      wstring_view infix = L"-x-";
      auto pos = language_name.find(infix);
      if (pos != wstring_view::npos)
        language_name = language_name.substr(0, pos);

      auto pos1 = language_name.find(L'-');
      if (pos1 == wstring_view::npos) {
        // neutral
        return find_locale_file(prefix, language_name, domain);
      }

      auto pos2 = language_name.find(L'-', pos1 + 1);
      if (pos2 == wstring_view::npos) {
        // ll-CC
        wstring gnu_name{language_name.data(), language_name.size()};
        gnu_name[pos1] = L'_';
        return find_locale_file(prefix, gnu_name, domain);
      } else {
        // ll-Script-CC
        // Win32 script name differs from GNU ones, ignore it.
        wstring gnu_name{language_name.data(), pos1};
        gnu_name.push_back(L'_');
        wstring_view region_code = language_name.substr(pos2 + 1);
        gnu_name.append(region_code);
        return find_locale_file(prefix, gnu_name, domain);
      }
    }

    file find_locale_file_from_win32(wstring_view prefix, string_view domain)
    {
      static auto pfn = (decltype(&GetUserPreferredUILanguages))GetProcAddress(
          GetModuleHandleW(L"kernel32.dll"), "GetUserPreferredUILanguages");
      if (!pfn)
        return {};

      wchar_t lang_buf[MAX_PATH];
      ULONG lang_buf_len = MAX_PATH;
      ULONG n_lang = 0;
      if (!pfn(MUI_LANGUAGE_NAME, &n_lang, lang_buf, &lang_buf_len))
        return {};

      wchar_t *p = lang_buf;
      while (*p) {
        wstring_view language_name = p;
        file fp = find_locale_file_by_win32_language_name(
            prefix, language_name, domain);
        if (fp)
          return fp;
        p += language_name.size() + 1;
      }

      return {};
    }

    file find_locale_file(wstring_view prefix,
                          wstring_view locale,
                          string_view domain)
    {
      // try full locale
      wstring path = compose_locale_file_path(prefix, locale, domain);
      printf("path = %ls\n", path.data());
      file fp(path.c_str(), L"rb");
      if (fp)
        return fp;

      // try without '.encoding' and '@variant'
      size_t pos = locale.find_first_of(L".@");
      if (pos != string::npos) {
        path = compose_locale_file_path(prefix, locale.substr(0, pos), domain);
        printf("path = %ls\n", path.data());
        fp = file(path.c_str(), L"rb");
        if (fp)
          return fp;
      }

      // try without '_CC'
      pos = locale.find(L'_');
      if (pos != string::npos) {
        path = compose_locale_file_path(prefix, locale.substr(0, pos), domain);
        printf("path = %ls\n", path.data());
        fp = file(path.c_str(), L"rb");
        if (fp)
          return fp;
      }

      return {};
    }

    wstring compose_locale_file_path(wstring_view prefix,
                                     wstring_view locale,
                                     string_view domain)
    {
      wstring_view infix = LR"(\LC_MESSAGES\)";
      wstring_view suffix = L".mo";

      wstring result{prefix.data(), prefix.size()};
      result.append(locale.data(), locale.size());
      result.append(infix.data(), infix.size());

      {
        size_t old_size = result.size();
        result.append(domain.size(), 0);
        for (size_t i = 0; i < domain.size(); ++i)
          result[old_size + i] = domain[i];
      }

      result.append(suffix.data(), suffix.size());
      return result;
    }
  };
} // namespace intl
