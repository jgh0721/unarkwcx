// Copyright 2008 Alexandre Courpron                                        
//                                                                          
// This file is part of the tek library.
// The tek library is licensed under the Code Project Open License v1.0.

#ifndef TEK_RECORD_RECORD_H
#define TEK_RECORD_RECORD_H

#include "tek_record_macros_defs.h"

#ifndef TEK_RECORD_NO_IO
#include <iostream>
#endif

namespace tek {

namespace private_impl {

namespace record {

// ******************
// record_data
// ******************

template < class T, class U >
struct record_data {
    typedef record_data< T, U > this_type;

    typedef T element_type;
    element_type element;

    typedef U next_type;
    next_type next; 

    enum { size = 1 + next_type::size };

    TEK_UNFOLD_RECORD_DATA_CTOR(TEK_RECORD_MAX_PARAMS)    

    record_data() {}

    this_type& operator=( const this_type& a )
    {
        if ( &a = this ) return *this;
        this->element = a.element;
        this->next = a.next;
        return *this;
    }
};

template < class T >
struct record_data< T, private_impl::utils::null_type > {
    typedef record_data< T, private_impl::utils::null_type > this_type;

    typedef T element_type;
    element_type element;

    typedef private_impl::utils::null_type next_type;

    enum { size = 1 };

    record_data( T a ) : element( a ) {}

    record_data() {}

    this_type& operator=( const this_type& a )
    {
        if ( &a = this ) return *this;
        this->element = a.element;
        return *this;
    }
};

// ******************
// for_interface
// ******************

template < int I, class B, bool IsDerived >
struct for_interface_struct {
    template < class T, class U >
    static void run( T& functor, U& data )
    {
        functor( data.element );
        for_interface_struct< I-1,
                            B,
                            private_impl::utils::is_derived_from< typename U::next_type::element_type,
                                        B >::check 
                          >::run( functor, data.next ); 
    }
};

template < int I, class B >
struct for_interface_struct < I, B, false > {
    template < class T, class U >
    static void run( T& functor, U& data ) 
    {
        for_interface_struct< I-1,
                            B,
                            private_impl::utils::is_derived_from< typename U::next_type::element_type,
                                            B >::check 
                          >::run( functor, data.next ); 
    }
};

template < class B >
struct for_interface_struct < 1, B, true > {
    template < class T, class U >
    static void run( T& functor, U& data ) 
    { 
        functor( data.element ); 
    }
};

template < class B >
struct for_interface_struct < 1, B, false > {
    template < class T, class U >
    static void run( T&, U& ) {}
};

// _helper function
template < class B, class T, class U >
inline void for_interface_helper( T& functor, U& data )
{
    for_interface_struct< U::size, 
                    B, 
                    private_impl::utils::is_derived_from< typename U::element_type,
                                   B >::check >::run( functor, data );
}

// ******************
// for_each
// ******************

template < int I >
struct for_each_struct {
    template < class T, class U >
    static void run( T& functor, U& data ) 
    { 
        functor( data.element );
        for_each_struct< I-1>::run( functor, data.next );
    }
};

template <>
struct for_each_struct< 1 > {
    template < class T, class U >
    static void run( T& functor, U& data ) 
    { 
        functor( data.element ); 
    }
};

// _helper function
template < int I, class T, class U >
inline void for_each_helper( T& functor, U& data )
{
    for_each_struct< I >::run( functor, data );
}

// ******************
// get_element
// ******************

template< int I, class R >
struct get_element {
    template< class T >
    static R run( T& data )
    {
        return get_element< I-1, R >::run( data.next );
    }
};

template< class R >
struct get_element< 0, R > {
    template< class T >
    static R run( T& data ) 
    {
        return data.element;
    }
};

//_helper function
template< int I, template < class > class ret_class, class T >
inline typename ret_class< typename tek::private_impl::utils::get_type< T, I >::type >::type
get_element_helper( T& data ) 
{
    return get_element< I, 
                       typename ret_class< typename tek::private_impl::utils::get_type< T, I >::type >::type
                     >::run( data ) ;
}

// ******************
// apply
// ******************

#ifdef TEK_RECORD_MERGE_APPLY

template < int I, template < class > class ret_class >
struct merge_apply_struct {
    template < class T, class U >
    static void run( void* const element, const unsigned int x, U& functor )
    {
        functor( *static_cast< typename ::tek::private_impl::utils::get_type< T, 0 >::type * >( element) );
    }
};

TEK_UNFOLD_RECORD_MERGE_APPLY_STRUCT(TEK_RECORD_MAX_PARAMS);

#endif //#ifdef TEK_RECORD_MERGE_APPLY


template < int I, template < class > class ret_class >
struct apply_struct {
    template < class T, class U >
    static void run( T& data, const unsigned int x, U& functor )
    {
        functor( get_element_helper< 0, ret_class >( data ) );
    }
};

TEK_UNFOLD_RECORD_APPLY_STRUCT(TEK_RECORD_MAX_PARAMS);

#ifdef TEK_RECORD_MERGE_APPLY
// ******************
// FillFunctor
// ******************

struct fill_addr_functor {
    typedef void** addr_table;
    fill_addr_functor ( addr_table tab )
        : tab_(tab), index(0) {}
    template <class T>
    void operator()(T& x)
    {
        tab_[index] = (void*)&x;
        index++;
    }
    addr_table tab_;
    int index;
private : 
    // no warning
    void operator=(const fill_addr_functor&);
};
#endif

#ifndef TEK_RECORD_NO_IO
// ******************
// print
// ******************

template < class T >
struct print {
    static void
    run( std::ostream& o, const T& data )
    {
        o << ' ' << data.element;
        print< typename T::next_type >::run( o, data.next );
    }
};

// TODO : remove coupling with record_data
template < class T >
struct print< record_data< T, private_impl::utils::null_type > > {
    static void
    run( std::ostream& o, const record_data< T, private_impl::utils::null_type >& data )
    {
      o << ' ' << data.element;
    }
};

//print_helper
template < class T >
inline static
void 
print_helper( std::ostream& o, const T& data )
{
    o << data.element;
    print< typename T::next_type >::run( o, data.next );
}

// ******************
// read
// ******************

template < class T >
struct read {
    static void
    run( std::istream& i, T& data )
    {
        i.ignore();
        i >> data.element;
        read< typename T::next_type >::run( i, data.next );
    }
};

// TODO : remove coupling with record_data
template < class T >
struct read< record_data< T, private_impl::utils::null_type > > {
    static void
    run( std::istream& i, record_data< T, private_impl::utils::null_type >& data )
    {
        i.ignore();
        i >> data.element;
    }
};

//read_helper
template < class T >
inline static void 
read_helper( std::istream& i, T& data )
{
    i >> data.element;
    read< typename T::next_type >::run( i, data.next );
}
#endif // #ifndef TEK_RECORD_NO_IO

// ******************
// strip_null_type
// ******************

// TODO : remove coupling with record_data and put it in utilities
template < class T, class U >
struct strip_null_type {
    typedef record_data< T, typename U::type > type;
};

template < class T, class U >
struct strip_null_type < T, strip_null_type< utils::null_type,U > >{
    typedef record_data< T, utils::null_type > type;
};
template < class T >
struct strip_null_type < T, utils::null_type >{
    typedef record_data< T, utils::null_type > type;
};

} // namespace record

} // namespace private_impl

// ******************
// record forward declaration
// ******************

template < TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
class record;

// ******************
// record operations
// ******************

template< int I,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_ref_type< 
         typename record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) >::template element<I>::type 
         >::type
get ( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg )
{
    return record_arg.template get< I >();
}

template< int I,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_ref_type< 
         typename record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) >::template element<I>::type 
         >::type
get ( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg )
{
    return record_arg.template get< I >();
}

template < class T >
inline
bool 
is_valid_index( const T& container, const unsigned int x )
{
    return x < container.size;
}

template < class U, 
           TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_return_type< U >::type
apply( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
       U& functor, 
       const unsigned int index )
{
    return record_arg.apply( functor, index );
} 

template< class U, 
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_return_type< U >::type
apply( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
       U& functor, 
       const unsigned int index )
{
    return record_arg.apply( functor, index );
}

template < class U, 
           TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_return_type< U >::type
apply( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
       const U& functor, 
       const unsigned int index )
{
    return record_arg.apply( functor, index );
} 

template< class U, 
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_return_type< U >::type
apply( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
       const U& functor, 
       const unsigned int index )
{
    return record_arg.apply( functor, index );
}

#ifdef TEK_RECORD_MERGE_APPLY

template < class U, 
           TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_return_type< U >::type
merge_apply( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
       U& functor, 
       const unsigned int index )
{
    return record_arg.merge_apply( functor, index );
} 

template< class U, 
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_return_type< U >::type
merge_apply( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
       U& functor, 
       const unsigned int index )
{
    return record_arg.merge_apply( functor, index );
}

template < class U, 
           TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_return_type< U >::type
merge_apply( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
       const U& functor, 
       const unsigned int index )
{
    return record_arg.merge_apply( functor, index );
} 

template< class U, 
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
typename private_impl::utils::get_return_type< U >::type
merge_apply( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
       const U& functor, 
       const unsigned int index )
{
    return record_arg.merge_apply( functor, index );
}

#endif //#ifdef TEK_RECORD_MERGE_APPLY

template< class U,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
void 
for_each( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
          U& functor )
{
    record_arg.for_each( functor );
}

template< class U,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
void 
for_each( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
          U& functor )
{
    record_arg.for_each( functor );
}

template< class U,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
void 
for_each( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
          const U& functor )
{
    record_arg.for_each( functor );
}

template< class U,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
void 
for_each( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
          const U& functor )
{
    record_arg.for_each( functor );
}

template< class Base, 
          class U,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
void 
for_interface( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
               U& functor )
{
    record_arg.template for_interface< Base >( functor );
}

template< class Base, 
          class U,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
void 
for_interface( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
               U& functor )
{
    record_arg.template for_interface< Base >( functor );
}

template< class Base, 
          class U,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
void 
for_interface( record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
               const U& functor )
{
    record_arg.template for_interface< Base >( functor );
}

template< class Base, 
          class U,
          TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline
void 
for_interface( const record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg,
               const U& functor )
{
    record_arg.template for_interface< Base >( functor );
}

// ******************
// record class
// ******************

template < TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NULLTYPE(TEK_RECORD_MAX_PARAMS) >
class record {

    typedef typename TEK_UNFOLD_RECORD_DATA_TYPELIST(TEK_RECORD_MAX_PARAMS) data_type;
    typedef record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > this_type;

public :
    enum { size = data_type::size };

    TEK_UNFOLD_RECORD_CTOR(TEK_RECORD_MAX_PARAMS)

    record() {}

    template< int I >
    struct element {
        typedef typename private_impl::utils::get_type< data_type, I >::type type;
    };


    template< int I >
    typename private_impl::utils::get_ref_type< typename element<I>::type >::type
    get () 
    {
        return private_impl::record::get_element_helper< I, private_impl::utils::get_ref_type > ( data_ );
    }

    template< int I >
    typename private_impl::utils::get_const_ref_type< typename element<I>::type >::type 
    get () const
    {
        return private_impl::record::get_element_helper< I, private_impl::utils::get_const_ref_type > ( data_ );
    }

    static bool is_valid_index( const unsigned int x )
    {
        return x < size;
    }

    template< class U >
    typename private_impl::utils::get_return_type< U >::type
    apply( U& functor, const unsigned int x )
    {
        return private_impl::record::apply_struct< size, private_impl::utils::get_ref_type >::run( data_, functor, x );
    } 

    template< class U >
    typename private_impl::utils::get_return_type< U >::type 
    apply( U& functor, const unsigned int x ) const
    {
        return private_impl::record::apply_struct< size, private_impl::utils::get_const_ref_type >::run( data_, functor, x );
    }

    template< class U >
    typename private_impl::utils::get_return_type< U >::type 
    apply( const U& functor, const unsigned int x )
    {
        return private_impl::record::apply_struct< size, private_impl::utils::get_ref_type >::run( data_, functor, x );
    } 

    template< class U >
    typename private_impl::utils::get_return_type< U >::type 
    apply( const U& functor, const unsigned int x ) const
    {
        return private_impl::record::apply_struct< size, private_impl::utils::get_const_ref_type >::run( data_, functor, x );
    }

#ifdef TEK_RECORD_MERGE_APPLY
    template< class U >
    typename private_impl::utils::get_return_type< U >::type 
    merge_apply( U& functor, const unsigned int x )
    {
        return private_impl::record::merge_apply_struct< size, private_impl::utils::get_ref_type >::template run< typename this_type::data_type >( element_addresses_[x], functor, x );
    } 

    template< class U >
    typename private_impl::utils::get_return_type< U >::type 
    merge_apply( U& functor, const unsigned int x ) const
    {
        return private_impl::record::merge_apply_struct< size, private_impl::utils::get_const_ref_type >::template run< typename this_type::data_type >( element_addresses_[x], functor, x );
    }

    template< class U >
    typename private_impl::utils::get_return_type< U >::type 
    merge_apply( const U& functor, const unsigned int x )
    {
        return private_impl::record::merge_apply_struct< size, private_impl::utils::get_ref_type >::template run< typename this_type::data_type >( element_addresses_[x], functor, x );
    } 

    template< class U >
    typename private_impl::utils::get_return_type< U >::type
    merge_apply( const U& functor, const unsigned int x ) const
    {
        return private_impl::record::merge_apply_struct< size, private_impl::utils::get_const_ref_type >::template run< typename this_type::data_type >( element_addresses_[x], functor, x );
    }
#endif // #ifdef TEK_RECORD_MERGE_APPLY

    template< class U > 
    void 
    for_each( U& functor )
    {
        private_impl::record::for_each_helper< size >( functor, data_ );
    }

    template< class U > 
    void 
    for_each( U& functor ) const
    {
        private_impl::record::for_each_helper< size >( functor, data_ );
    }

    template< class U > 
    void 
    for_each( const U& functor )
    {
        private_impl::record::for_each_helper< size >( functor, data_ );
    }

    template< class U > 
    void 
    for_each( const U& functor ) const
    {
        private_impl::record::for_each_helper< size >( functor, data_ );
    }

    template< class Base, class U >
    void 
    for_interface( U& functor )
    {
        private_impl::record::for_interface_helper< Base >( functor, data_ );
    }
   
    template< class Base, class U >
    void 
    for_interface( U& functor ) const
    {
        private_impl::record::for_interface_helper< Base >( functor, data_ );
    }

    template< class Base, class U >
    void 
    for_interface( const U& functor )
    {
        private_impl::record::for_interface_helper< Base >( functor, data_ );
    }
   
    template< class Base, class U >
    void 
    for_interface( const U& functor ) const
    {
        private_impl::record::for_interface_helper< Base >( functor, data_ );
    }

#ifndef TEK_RECORD_NO_IO
    std::ostream& 
    print( std::ostream& o ) const
    {
        o << '(';
        private_impl::record::print_helper( o, data_ );
        o << ')';
        return o;
    }

    std::istream& 
    read( std::istream& i )
    {
        i.ignore();
        private_impl::record::read_helper( i, data_ );
        i.ignore();
        return i;
    }
#endif // #ifndef TEK_RECORD_NO_IO

private :    
    data_type data_;

#ifdef TEK_RECORD_MERGE_APPLY
    void* element_addresses_[size];

    void fill_addr () 
    {
        private_impl::record::fill_addr_functor fill( element_addresses_ );
        private_impl::record::for_each_helper< size >( fill , data_ );
    }
#endif // #ifdef TEK_RECORD_MERGE_APPLY
};

} // namespace tek

#ifndef TEK_RECORD_NO_IO

template< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline 
std::ostream& 
operator<<( std::ostream& o,
           const tek::record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg )
{
    return record_arg.print( o );
}

template< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS(TEK_RECORD_MAX_PARAMS) >
inline 
std::istream& 
operator>>( std::istream& i,
           tek::record< TEK_UNFOLD_RECORD_TEMPLATE_PARAMS_NAME(TEK_RECORD_MAX_PARAMS) > & record_arg )
{
    return record_arg.read( i );
}

#endif // #ifndef TEK_RECORD_NO_IO

#include "tek_record_macros_undefs.h"

#endif // #ifndef TEK_RECORD_RECORD_H
