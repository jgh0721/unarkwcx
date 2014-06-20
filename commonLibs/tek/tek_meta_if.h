// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.


#ifndef TEK_UTILS_META_IF_H
#define TEK_UTILS_META_IF_H

namespace tek {

namespace private_impl {

namespace utils {

template< bool condition, class then_, class else_ >
struct meta_if
{
    typedef then_ type;  
};

template < class then_, class else_ >
struct meta_if < false, then_, else_ >
{
    typedef else_ type;  
};


} // namespace utils

} // namespace private_impl

} // namespace tek

#endif // #ifndef TEK_UTILS_META_IF_H