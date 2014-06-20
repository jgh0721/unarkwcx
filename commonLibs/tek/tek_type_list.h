// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.


#ifndef TEK_UTILS_TYPE_LIST_H
#define TEK_UTILS_TYPE_LIST_H

namespace tek {

namespace private_impl {

namespace utils {

// ******************
// null_type
// ******************

struct null_type {
    enum { size = 0 };
};

// ******************
// get_type
// ******************

template< class T, int I >
struct get_type {
    typedef typename get_type< typename T::next_type, 
                              I-1 >::type type;
};

template< class T >
struct get_type< T, 0 > {
    typedef typename T::element_type type;
};

// ******************
// get_type_index
// ******************

template< class T, class U >
struct get_type_index {
    enum { index = 1 + get_type_index< typename T::next_type, 
                                     U >::index };
};

template< class T >
struct get_type_index< T, typename T::element_type > {
    enum { index = 0 };
};

// ******************
// silent_get_type
// ******************

template< class T, int I >
struct silent_get_type {
    typedef typename silent_get_type< typename T::next_type, 
                                    I-1 >::type type;
};

template< class T >
struct silent_get_type< T, 0 > {
    typedef typename T::element_type type;
};

template< int I >
struct silent_get_type< null_type, I > {
    typedef null_type type;
};

template <>
struct silent_get_type< null_type, 0 > {
    typedef null_type type;
};

} // namespace utils

} // namespace private_impl

} // namespace tek

#endif // #ifndef TEK_UTILS_TYPE_LIST_H
