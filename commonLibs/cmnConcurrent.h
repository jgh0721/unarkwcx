#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <ios>
#include <list>
#include <queue>
#include <vector>
#include <xutility>
#include <unordered_map>

#include <windows.h>

#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

#include "cmnUtils.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    class CCriticalSectionEx
    {
    public:
        CCriticalSectionEx() { InitializeCriticalSectionAndSpinCount( &m_cs, 4000 ); };
        ~CCriticalSectionEx() { DeleteCriticalSection( &m_cs ); };

        void Lock() { EnterCriticalSection( &m_cs ); };
        void Unlock() { LeaveCriticalSection( &m_cs ); };

    private:
        CRITICAL_SECTION        m_cs;
    };

    class CScopedCriticalSection
    {
    public:
        explicit CScopedCriticalSection( CRITICAL_SECTION& cs );
        ~CScopedCriticalSection();

    private:
        CRITICAL_SECTION&		m_cs;
    };

    //////////////////////////////////////////////////////////////////////////

    template< typename T >
    class CLocker
    {
        T& _obj;
    public:
        CLocker( T& obj ) : _obj( obj )
        {
            _obj.Lock();
        }

        ~CLocker()
        {
            _obj.Unlock();
        }
    };


    template< typename T >
    class CSharedQueue
    {
        CRITICAL_SECTION                _cs;
        boost::condition_variable       _cv;
        std::queue<T>                   _queue;

    public:
        CSharedQueue() { ::InitializeCriticalSectionAndSpinCount( &_cs, 4000 ); };
        ~CSharedQueue() { ::DeleteCriticalSection( &_cs ); };

        void Push( const T& item )
        {
            CScopedCriticalSection lck( _cs );
            _queue.push( item );
            _cv.notify_one();
        };

#if _MSC_VER >= 1600 
        void Push( T&& item )
        {
            CScopedCriticalSection lck( _cs );
            _queue.emplace( item );
            _cv.notify_one();
        }
#endif

        bool Try_and_pop( T& poppedItem )
        {
            CScopedCriticalSection lck( _cs );
            if( _queue.empty() == true )
                return false;
#if _MSC_VER >= 1600 
            poppedItem = std::move( _queue.front() );
#else
            poppedItem = _queue.front();
#endif
            _queue.pop();
            return true;
        }

        void Wait_and_pop( T& poppedItem )
        {
            CScopedCriticalSection lck( _cs );
            while( _queue.empty() ) { ::SleepEx( 1, FALSE ); }
#if _MSC_VER >= 1600 
            poppedItem = std::move( _queue.front() );
#else
            poppedItem = _queue.front();
#endif
            _queue.pop();
        }

        bool IsEmpty() const
        {
            CScopedCriticalSection lck( _cs );
            return _queue.empty();
        }

        size_t Size() const
        {
            CScopedCriticalSection lck( _cs );
            return _queue.size();
        }

    };

    /*!
    CCmnObject 를 상속받은 객체들을 CCmnObjectHolder 를 통해 CCmnObjectPool 로 관리

    http://www.codeproject.com/Articles/8108/Template-based-Generic-Pool-using-C
    */
    class CCmnObject
    {
    public:
        CCmnObject( );
        ~CCmnObject( );

        virtual void Init( ) {};
        virtual void Release( ) {};
        virtual bool IsUsable( ) { return true; }

        virtual bool MakeUsable( )
        {
            if( !IsUsable( ) ) { Init( ); }
            return true;
        }
    };

    template< typename T >
    class CCmnObjectHolder
    {
    public:
        CCmnObjectHolder( ) : _pObj( NULLPTR ), _nTimeStamp( -1 ) {};
        ~CCmnObjectHolder( ) { ReleasePtr< T* >( _pObj ); };

        void InitObject( )
        {
            if( _pObj == NULLPTR )
            {
                _pObj = new T( );
                _pObj->Init( );
            }
        }

        T* GetObj( ) { return _pObj; };
        void SetObj( T* object = NULLPTR ) { _pObj = object; };

        long GetTimeStamp( ) { return _nTimeStamp; };
        void SetTimeStamp( long nTime ) { _nTimeStamp = nTime; };

        void Set( T* pObj, long nTime ) { _pObj = pObj; _nTimeStamp = nTime; };
    private:
        T*      _pObj;
        long    _nTimeStamp;
    };

    template< typename T >
    class CCmnObjectPool
    {
        typedef CCmnObjectHolder< T >       TyObjectHolder;
        typedef std::list< TyObjectHolder > TyObjectHolderList;

        static CCriticalSectionEx           _csPool;
        CCriticalSectionEx                  _csData;
    public:
        CCmnObjectPool( )
        { };

        ~CCmnObjectPool( )
        { };


        void                                InitPool( unsigned int nMaxPoolSize, long unsigned int nWaitTime = 3 )
        {
            _nMaxPoolSize = nMaxPoolSize;
            _nWaitTime = nWaitTime;
        }

    private:
        T*                                  createObject( )
        {
            return new T( );
        }

        T*                                  findObject( )
        {
            return new T( );
        }

        unsigned int                        _nMaxPoolSize;
        unsigned int                        _nWaitTime;
        TyObjectHolderList                  _lstReservedObj;
        TyObjectHolderList                  _lstFreeObj;

    };

    template< typename T >
    class CRoundRobinAllocator
    {
    public:
        CRoundRobinAllocator( size_t maxPoolSize = 256 )
            : _lCurrentIndex( 0 ), _poolSize( maxPoolSize )
        {
            ::InitializeCriticalSectionAndSpinCount( &_cs, 4000 );
        }

        ~CRoundRobinAllocator()
        {
            ::DeleteCriticalSection( &_cs );
        }

        void CreatePool()
        {
            CScopedCriticalSection lck( _cs );

            for( size_t idx = 0; idx < _poolSize; ++idx )
            {
                _vecPool.push_back( new T() );
                _mapUsingTable[ idx ] = false;
            }
        }

        void DeletePool()
        {
            CScopedCriticalSection lck( _cs );

            for( size_t idx = 0; idx < _vecPool.size(); ++idx )
                delete _vecPool[ idx ];
        }

        T*  CheckOut()
        {
            CScopedCriticalSection lck( _cs );

            if( _lCurrentIndex >= _poolSize )
                _lCurrentIndex = 0;

            if( _mapUsingTable[ _lCurrentIndex ] == true )
                return NULLPTR;

            _mapUsingTable[ _lCurrentIndex ] = true;
            return _vecPool[ _lCurrentIndex++ ];
        }

        void CheckIn( T* object )
        {
            CScopedCriticalSection lck( _cs );
            for( size_t idx = 0; idx < _poolSize; ++idx )
            {
                if( _vecPool[ idx ] == object )
                {
                    _mapUsingTable[ idx ] = false;
                    break;
                }
            }
        }

    private:
        size_t                                  _poolSize;
        CRITICAL_SECTION                        _cs;
        std::vector< T* >                       _vecPool;

#if _MSC_VER >= 1600
        std::unordered_map< size_t, bool >      _mapUsingTable;
#else
        std::tr1::unordered_map< size_t, bool >      _mapUsingTable;
#endif      

        size_t                                  _lCurrentIndex;
    };

    template< typename T >
    class CConcurrentQueue : public boost::noncopyable
    {
    public:
        CConcurrentQueue() : _isStop( false ) {}

        void pop( T& val )
        {
            boost::unique_lock< boost::mutex > lck( _mutex );
            while( _queue.empty() && _isStop == false )
                _cvCond.wait( lck );

            if( _isStop == true )
                return;

            val = _queue.front();
            _queue.pop();
        }

        void push( const T& item )
        {
            boost::unique_lock< boost::mutex > lck( _mutex );
            _queue.push( item );
            lck.unlock();
            _cvCond.notify_one();
        }

#if _MSC_VER >= 1600 
        void push( T&& item )
        {
            boost::unique_lock< boost::mutex > lck( _mutex );
            _queue.push( item );
            lck.unlock();
            _cvCond.notify_one();
        }
#endif

        void stopAllPop()
        {
            _isStop = true;
            _cvCond.notify_all();
        }

        size_t getSize()
        {
            boost::unique_lock< boost::mutex > lck( _mutex );
            return _queue.size();
        }

        bool isEmpty()
        {
            return _queue.empty();
        }

    private:
        volatile bool               _isStop;
        std::queue< T >             _queue;
        boost::mutex                _mutex;
        boost::condition_variable   _cvCond;
    };

}