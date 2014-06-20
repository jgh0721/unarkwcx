// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.



// This file contains all preprocessor macros related to the record implementation
// #undefs are performed in the file record_macros_undefs.
// This organization may change when the software reaches a stable state.


#include "tek_compiler_migration.h"
#include "tek_repetition.h"
#include "tek_expression.h"
#include "tek_inc.h"
#include "tek_dec.h"
#include "tek_record_config.h"
#include "tek_type_transformation.h"
#include "tek_type_list.h"
#include "tek_is_derived_from.h"

#define TEK_PP_RECORD_DATA_LIST(n) private_impl::record::strip_null_type< T##n,

#define TEK_PP_RECORD_DATA_LIST_END(n) private_impl::utils::null_type TEK_PP_REPEAT(n,>)::type

#define TEK_PP_RECORD_DATA_CTOR_PARAM(n) ,typename private_impl::utils::get_param_type< typename private_impl::utils::silent_get_type< U, n >::type >::type a##n

#define TEK_PP_RECORD_DATA_CTOR(n) record_data( T a TEK_PP_NESTED_ITER(n,TEK_PP_RECORD_DATA_CTOR_PARAM) ) \
                                   : element( a ),                          \
                                   next( TEK_PP_NESTED_LIST(n,TEK_PP_ARG) ) {}

#define TEK_PP_RECORD_PARAMS(n) TEK_PP_NESTED_LIST(n,TEK_PP_PARAM)

#define TEK_PP_RECORD_M_DATA_ARGS(n) TEK_PP_NESTED_LIST(n,TEK_PP_ARG)

#ifdef TEK_RECORD_MERGE_APPLY
#define TEK_PP_RECORD_CTOR(n) record( TEK_PP_RECORD_PARAMS( TEK_PP_INC(n) ) ) \
                              : data_( TEK_PP_RECORD_M_DATA_ARGS( TEK_PP_INC(n) ) ) \
                              {fill_addr ();}
#else
#define TEK_PP_RECORD_CTOR(n) record( TEK_PP_RECORD_PARAMS( TEK_PP_INC(n) ) ) \
                              : data_( TEK_PP_RECORD_M_DATA_ARGS( TEK_PP_INC(n) ) ) \
                              {}
#endif

#ifdef TEK_RECORD_MERGE_APPLY
#define TEK_PP_RECORD_MERGE_APPLY(n) return functor( *(typename private_impl::utils::get_type< T, n >::type *)( element ) );
#endif
#define TEK_PP_RECORD_APPLY(n) return functor( get_element_helper< n, ret_class >( data ) );

#ifdef TEK_RECORD_MERGE_APPLY
#define TEK_PP_RECORD_MERGE_CASE(n) case n : TEK_PP_RECORD_MERGE_APPLY(n) break;
#endif
#define TEK_PP_RECORD_CASE(n) case n : TEK_PP_RECORD_APPLY(n) break;

#ifdef TEK_RECORD_MERGE_APPLY
#define TEK_PP_RECORD_MERGE_APPLY_STRUCT(n) \
template < template < class > class ret_class >\
struct merge_apply_struct< n, ret_class > {\
    template < class T, class U >\
    static typename private_impl::utils::get_return_type< U >::type\
    run( void* const element, U& functor, const unsigned int x )\
    {\
        switch (x) {\
            TEK_PP_NESTED_ITER(n,TEK_PP_RECORD_MERGE_CASE)\
            default : UNREACHABLE;\
        }\
    }\
};

#endif //#ifdef TEK_RECORD_MERGE_APPLY

#define TEK_PP_RECORD_APPLY_STRUCT(n)\
template < template < class > class ret_class >\
struct apply_struct< n, ret_class > {\
    template < class T, class U >\
    static typename private_impl::utils::get_return_type< U >::type\
    run( T& data, U& functor, const unsigned int x )\
    {\
        switch (x) {\
            TEK_PP_NESTED_ITER(n,TEK_PP_RECORD_CASE)\
            default : UNREACHABLE;\
        }\
    }\
};

// Macro wrappers : TEK_RECORD_MAX_PARAMS substituted before token-pasted

#define TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NULLTYPE(n) TEK_PP_LIST(n,TEK_PP_TEMPLATE_PARAM_NULLTYPE)

#define TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(n) TEK_PP_LIST(n,TEK_PP_TEMPLATE_PARAM)

#define TEK_UNFOLD_RECORD_DATA_TYPELIST(n) TEK_PP_ITER(n,TEK_PP_RECORD_DATA_LIST) \
                                           TEK_PP_RECORD_DATA_LIST_END(n)

#define TEK_UNFOLD_RECORD_CTOR(n) TEK_PP_ITER(n,TEK_PP_RECORD_CTOR)

#define TEK_UNFOLD_RECORD_DATA_CTOR(n) TEK_PP_ITER(n,TEK_PP_RECORD_DATA_CTOR)

#define TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(n) TEK_PP_LIST(n,TEK_PP_TEMPLATE_PARAM_NAME)

#define TEK_UNFOLD_RECORD_APPLY_STRUCT(n) TEK_PP_ITER(TEK_PP_INC(n),TEK_PP_RECORD_APPLY_STRUCT)

#define TEK_UNFOLD_RECORD_MERGE_APPLY_STRUCT(n) TEK_PP_ITER(TEK_PP_INC(n),TEK_PP_RECORD_MERGE_APPLY_STRUCT)
