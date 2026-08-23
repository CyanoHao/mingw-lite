#pragma once

#include "corecrt_internal.h"

#include <errno.h>
#include <stdlib.h>

namespace mingw_thunk::ucrt
{

  class __crt_cached_ptd_host
  {
  public:
    explicit __crt_cached_ptd_host(_locale_t const locale = nullptr) noexcept
    {
    }

    ~__crt_cached_ptd_host() noexcept
    {
      if (_current_errno.valid())
      {
        _set_errno(_current_errno.unsafe_value());
      }
      if (_current_doserrno.valid())
      {
        _set_doserrno(_current_doserrno.unsafe_value());
      }
    }

    _locale_t get_locale() throw()
    {
      return _LocaleUpdate::g_locale_c();
    }

    template <typename T>
    struct cached
    {
    public:
      cached() noexcept
          : _valid(false)
      {
      }

      bool valid() const noexcept
      {
        return _valid;
      }

      T set(T new_value) noexcept
      {
        _valid = true;
        _value = new_value;
        return new_value;
      }

      T value_or(T const alternative) const noexcept
      {
        if (_valid)
        {
          return _value;
        }
        return alternative;
      }

      bool check(T const value) const noexcept
      {
        return _valid && _value == value;
      }

      class guard
      {
      public:
        explicit guard(cached &parent) noexcept
            : _parent(parent)
            , _copy(parent)
            , _enabled(true)
        {
        }

        ~guard() noexcept
        {
          if (_enabled)
          {
            _parent = _copy;
          }
        }

        guard(guard const &) = delete;
        guard &operator=(guard const &) = delete;

        void disable() noexcept
        {
          _enabled = false;
        }

        void enable() noexcept
        {
          _enabled = true;
        }

      private:
        cached &_parent;
        cached _copy;
        bool _enabled;
      };

      guard create_guard() noexcept
      {
        return guard(*this);
      }

      T unsafe_value() throw()
      {
        return _value;
      }

    private:
      cached(cached const &) = default;
      cached(cached &&) = default;

      cached &operator=(cached const &) = default;
      cached &operator=(cached &&) = default;

      T _value;
      bool _valid;
    };

    auto &get_errno() throw()
    {
      return _current_errno;
    }

    auto &get_doserrno() throw()
    {
      return _current_doserrno;
    }

  private:
    cached<errno_t> _current_errno;
    cached<unsigned long> _current_doserrno;
  };

#define _UCRT_VALIDATE_RETURN(ptd, expr, errorcode, retexpr)                   \
  {                                                                            \
    int _Expr_val = !!(expr);                                                  \
    if (!(_Expr_val))                                                          \
    {                                                                          \
      (ptd).get_errno().set((errorcode));                                      \
      return (retexpr);                                                        \
    }                                                                          \
  }

#define _UCRT_VALIDATE_RETURN_ERRCODE(ptd, expr, errorcode)                    \
  {                                                                            \
    int _Expr_val = !!(expr);                                                  \
    if (!(_Expr_val))                                                          \
    {                                                                          \
      (ptd).get_errno().set((errorcode));                                      \
      return (errorcode);                                                      \
    }                                                                          \
  }

#define _UCRT_VALIDATE_RETURN_NOEXC(ptd, expr, errorcode, retexpr)             \
  {                                                                            \
    if (!(expr))                                                               \
    {                                                                          \
      (ptd).get_errno().set((errorcode));                                      \
      return (retexpr);                                                        \
    }                                                                          \
  }

} // namespace mingw_thunk::ucrt
