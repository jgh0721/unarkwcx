// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.

#ifndef TEK_PREPROCESSOR_EXPRESSION_H
#define TEK_PREPROCESSOR_EXPRESSION_H

#define TEK_PP_TEMPLATE_PARAM(n) class T##n
#define TEK_PP_TEMPLATE_PARAM_NAME(n) T##n
#define TEK_PP_TEMPLATE_PARAM_NULLTYPE(n) class T##n = ::tek::private_impl::utils::null_type
#define TEK_PP_PARAM(n) typename ::tek::private_impl::utils::get_param_type< T##n >::type a##n
#define TEK_PP_ARG(n) a##n

#endif // #ifndef TEK_PREPROCESSOR_EXPRESSION_H
