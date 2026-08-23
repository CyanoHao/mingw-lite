#pragma once

namespace mingw_thunk::ucrt
{

#define _calloc_crt calloc
#define _free_crt free
#define _malloc_crt malloc
#define _realloc_crt realloc
#define _msize_crt _msize
#define _recalloc_crt _recalloc

  template <typename T>
  class __crt_unique_heap_ptr
  {
  public:
    explicit __crt_unique_heap_ptr(T *const p = nullptr) noexcept
        : _p(p)
    {
    }

    __crt_unique_heap_ptr(__crt_unique_heap_ptr const &) = delete;

    __crt_unique_heap_ptr &operator=(__crt_unique_heap_ptr const &) = delete;

    __crt_unique_heap_ptr(__crt_unique_heap_ptr &&other) noexcept
        : _p(other._p)
    {
      other._p = nullptr;
    }

    ~__crt_unique_heap_ptr() noexcept
    {
      release();
    }

    __crt_unique_heap_ptr &operator=(__crt_unique_heap_ptr &&other) noexcept
    {
      release();
      _p = other._p;
      other._p = nullptr;
      return *this;
    }

    T *detach() noexcept
    {
      T *const local_p{_p};
      _p = nullptr;
      return local_p;
    }

    void attach(T *const p) noexcept
    {
      release();
      _p = p;
    }
    void release() noexcept
    {
      free(_p);
      _p = nullptr;
    }

    bool is_valid() const noexcept
    {
      return _p != nullptr;
    }

    explicit operator bool() const noexcept
    {
      return is_valid();
    }

    T *get() const noexcept
    {
      return _p;
    }

    T **get_address_of() noexcept
    {
      return &_p;
    }

    T **release_and_get_address_of() noexcept
    {
      release();
      return &_p;
    }

  private:
    T *_p;
  };

#define _calloc_crt_t(t, n)                                                    \
  (__crt_unique_heap_ptr<t>(static_cast<t *>(_calloc_crt((n), sizeof(t)))))
#define _malloc_crt_t(t, n)                                                    \
  (__crt_unique_heap_ptr<t>(static_cast<t *>(_malloc_crt((n) * sizeof(t)))))
#define _recalloc_crt_t(t, p, n)                                               \
  (__crt_unique_heap_ptr<t>(                                                   \
      static_cast<t *>(_recalloc_crt((p), (n), sizeof(t)))))

#ifdef _DEBUG
#define _INVALID_PARAMETER(expr)                                               \
  _invalid_parameter(expr, __FUNCTIONW__, __FILEW__, __LINE__, 0)
#else
#define _INVALID_PARAMETER(expr) _invalid_parameter_noinfo()
#endif

#define _VALIDATE_RETURN(expr, errorcode, retexpr)                             \
  {                                                                            \
    int _Expr_val = !!(expr);                                                  \
    if (!(_Expr_val))                                                          \
    {                                                                          \
      errno = errorcode;                                                       \
      return retexpr;                                                          \
    }                                                                          \
  }

#define _VALIDATE_RETURN_VOID(expr, errorcode)                                 \
  {                                                                            \
    int _Expr_val = !!(expr);                                                  \
    _ASSERT_EXPR((_Expr_val), _CRT_WIDE(#expr));                               \
    if (!(_Expr_val))                                                          \
    {                                                                          \
      errno = errorcode;                                                       \
      _INVALID_PARAMETER(_CRT_WIDE(#expr));                                    \
      return;                                                                  \
    }                                                                          \
  }

} // namespace mingw_thunk::ucrt
