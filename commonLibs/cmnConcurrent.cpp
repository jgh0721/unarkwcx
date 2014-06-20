#include "stdafx.h"

#include "cmnConcurrent.h"

#pragma execution_character_set( "utf-8" )

namespace nsCommon
{
    CScopedCriticalSection::CScopedCriticalSection( CRITICAL_SECTION& cs )
        : m_cs( cs )
    {
        EnterCriticalSection( &m_cs );
    }

    CScopedCriticalSection::~CScopedCriticalSection()
    {
        LeaveCriticalSection( &m_cs );
    }

    

}