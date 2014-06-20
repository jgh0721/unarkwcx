// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.

#ifndef TEK_UTILS_TYPE_TRANSFORMATION_H
#define TEK_UTILS_TYPE_TRANSFORMATION_H

#include "tek_is_derived_from.h"
#include "tek_meta_if.h"
#include "tek_return_type.h"

namespace tek {

namespace private_impl {

namespace utils {

// ******************
// get_ref_type
// ******************

template < class T >
struct get_ref_type {
    typedef T& type;
};

template < class T >
struct get_ref_type< T& > {
    typedef T& type;
};

// ******************
// get_const_ref_type
// ******************

template < class T >
struct get_const_ref_type {
    typedef const T& type;
};

template < class T >
struct get_const_ref_type< T& > {
    typedef const T& type;
};

template < class T >
struct get_const_ref_type< const T > {
    typedef const T& type;
};

template < class T >
struct get_const_ref_type< const T& > {
    typedef const T& type;
};

// ******************
// get_param_type
// ******************

template < class T >
struct get_param_type {
    typedef const T& type;
};

template < class T >
struct get_param_type< T& > {
    typedef T& type;
};

template < class T >
struct get_param_type< const T > {
    typedef const T& type;
};

template < class T >
struct get_param_type< const T& > {
    typedef const T& type;
};

// ******************
// get_return_type
// ******************

template < class T >
struct get_return_type {
    typedef typename meta_if < is_derived_from< T, return_type_base >::check,
                               T,
                               tek::return_type< void > >::type::hidden_return_type type;                        
};

template < class T >
struct get_return_type < const T > {
    typedef typename meta_if < is_derived_from< T, const return_type_base >::check,
                               T,
                               tek::return_type< void > >::type::hidden_return_type type;                        
};

} // namespace utils

} // namespace private_impl

} // namespace tek

#endif // #ifndef TEK_UTILS_TYPE_TRANSFORMATION_H
