// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.

#ifndef TEK_PREPROCESSOR_COMPILERMIGRATION_H
#define TEK_PREPROCESSOR_COMPILERMIGRATION_H

#ifdef _MSC_VER
#if _MSC_VER>=1310
    #define UNREACHABLE __assume(0);
#endif
#else
    #define UNREACHABLE
#endif

#endif // #ifndef TEK_PREPROCESSOR_COMPILERMIGRATION_H
