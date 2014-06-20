// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.

//defines the maximum number of types Record can handle
#ifndef TEK_RECORD_MAX_PARAMS
    #define TEK_RECORD_MAX_PARAMS 20
#endif

//disables iostream operators suppport (prevents the inclusion of <iostream> )
//#define TEK_RECORD_NO_IO 

//specifies a different implementation of the apply function
//can lead to performance improvement or degradation
//if unsure, don't define it
//#define TEK_RECORD_MERGE_APPLY
