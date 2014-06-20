// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.

#ifndef TEK_UTILS_RETURN_TYPE_H
#define TEK_UTILS_RETURN_TYPE_H

namespace tek {

namespace private_impl {

namespace utils {

struct return_type_base {};

} // namespace utils

} // namespace private_impl

template < class T >
struct return_type : public tek::private_impl::utils::return_type_base {
    typedef T hidden_return_type;
};

} // namespace tek

#endif // #ifndef TEK_UTILS_RETURN_TYPE_H