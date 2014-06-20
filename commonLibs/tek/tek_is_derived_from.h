// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.

#ifndef TEK_UTILS_IS_DERIVED_FROM_H
#define TEK_UTILS_IS_DERIVED_FROM_H

namespace tek {

namespace private_impl {

namespace utils {

template< class D, class B >
struct is_derived_from
{
    typedef char Derived;
    struct NotDerived{ 
      char Derived[2]; 
    };
    
    static Derived Test( B* );
    static NotDerived Test( ... );

    enum { check = sizeof( Test( static_cast<D*>(0) ) ) 
                   ==
                   sizeof( Derived ) };
};

} // namespace utils

} // namespace private_impl

} // namespace tek

#endif // #ifndef TEK_UTILS_IS_DERIVED_FROM_H
