============================
UnArkWCX for TotalCommander
=============================  

UnArkWCX is a WCX plugin that unpack various file formats using Ark Library. 

Author : DevTester ( http://jgh0721.homeip.net )

Development Environment
=========================

  * Visual Studio 2012( Use Visual Studio 2008 SP1 Compiler, So need VS2008 SP1 Installation )
  
  * Ark Library ( http://www.bandisoft.com/ark/ )
  
  * Total Commadner 8.X

History
========

  최종 수정 일 : 2012-10-XX

  참조 : Kipple 님의 Unarkwcx

1.0 ( 2012-10-XX )
--------------------

  * Ark Library 5.0 Beta 2 로 변경
  * 압축 파일 생성 지원 시작
  * 환경설정 대화상자 추가

  * 압축 파일 생성 ( ZIP, 7Z, ISO )
  * 압축 파일 해제 ( NSIS-EXE 추가 지원 )
    	
  * 플러그인을 설치할 때 모든 확장자를 등록하지 못하고 토탈 커맨더가 종료되는 문제 수정
  
  * 토탈 커맨더의 기본 플러그인 설치 인터페이스는 각 확장자별로 압축 또는 해제를 지정할 수 없는 관계로 별도의 설치관리자 제작
  * 토탈 커맨더에서 플러그인이 지원하는 기능을 정확히 파악하지 못하는 문제 수정
  * 폴더가 포함된 압축파일을 열 때 오류가 발생하는 현상 수정
  
  * 단위테스트를 위해 GoogleTest 1.6.0 도입
  * 압축 한 후 원본 파일 삭제 기능 구현
  * Visual Studio 2012 로 솔루션 파일 변경, 빌드는 VS 2008 SP1 을 사용하여 빌드함

0.9 ( 2012-10-XX )
--------------------

  * Ark Library 5.0 Beta 1 로 변경
  * Visual Studio 2008 SP1 프로젝트로 변경, CRT 에 의존하지 않도록 변경
  * 파일 날짜를 잘 못 반환하여 발생하는 오류 수정
  * 압축 해제 중 취소 눌렀을 때 취소 가능하도록 변경
  
0.5 ( 2011-10-03 )
--------------------

  * 해당 플러그인을 통해서 작업한 파일이 계속 LOCK 를 소유하는 문제 수정

0.4 ( 2011-10-02 )
--------------------

  * x64 지원 추가

0.3 ( 2011-10-02 )
--------------------

  * GetPackerCaps 구현
  * CanYouHandleThisFile 구현

  
0.2 ( 2011-10-01 )
--------------------

  * Ark Library 의 ExtractOneAs 메소드의 우회책 작성
  
0.1 ( 2011-10-01 )
---------------------

  * wcxtest 유틸리티의 시험 통과 ( -f, -l, -x )

  
