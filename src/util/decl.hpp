#pragma once

#define __disallow_construct(_C)     _C() = delete;

#define __disallow_nonconst_copy(_C) _C(_C &) = delete;
#define __disallow_const_copy(_C)    _C(const _C &) = delete;

#define __disallow_copy(_C) \
    __disallow_nonconst_copy(_C); \
    __disallow_const_copy(_C);

#define __disallow_nonconst_assign(_C) _C &operator= (_C &) = delete;
#define __disallow_const_assign(_C)    _C &operator= (const _C &) = delete;

#define __disallow_assign(_C) \
    __disallow_nonconst_assign(_C); \
    __disallow_const_assign(_C);
