;
; KaKeeware cmdTotal is yet another upgrade to IETOTALX proggie.
; http://www.kakeeware.com/i_ietotalx.php
; Development continued by Konstantin Vlasov a.k.a. CaptainFlint.
;
; This is a generic approach to make it possible to use any
; Total Commander WCX packer plugins from a command line.
;
; -- How to use? --
; From a command line, run:
;    cmdTotal [wcxPluginName] [option] [fileName] [targetDirectory]
;     where:
;       [wcxPluginName] - WCX plugin filename (path not needed if in the same directory)
;       [option] - can be:
;         l - to list the files
;         t - to test the files
;         x - to extract the files
;       [fileName] - file you want to process
;       [targetDirectory] - where to extract the files (default: _[fileName])
;
; Examples:
;     cmdTotal.exe InstExpl.wcx l WinPcap_3_1.exe             - to list the files inside the archive
;     cmdTotal.exe InstExpl.wcx x WinPcap_3_1.exe             - to extract the files inside the archive to _WinPcap_3_1.exe directory
;     cmdTotal.exe InstExpl.wcx x WinPcap_3_1.exe c:\unpacked - to extract the files inside the archive to c:\unpacked directory
;
; -- Technical note --
; There's a guard thread created at the beginning of the main routine that will kill the instance
; of cmdTotal, if the main thread haven't finished its work after 300 seconds.
;
; There's SEH handler added before calling the function that extracts the files to make sure
; we extract as many of them as possible.
;
; -- History --
;  2013-04-23 v. 2.02 - /by K.V./ further improvement of TC behavior emulation:
;                     -   a) for subdirectories calling ProcessFile(archiveHandle, PK_SKIP, NULL, NULL)
;                     -   b) always calling ProcessFile with DestPath == NULL and DestName == full target path
;  2013-04-17 v. 2.01 - /by K.V./ improved TC behavior emulation:
;                     -   a) providing ANSI ProcessDataProc to Unicode plugins too
;                     -   b) use ProcessFile with PK_SKIP for packed directories, create them ourselves
;                     - fixed destination path construction when it's omitted and archive is specified with path
;  2013-04-03 v. 2.00 - /by K.V./ added support for Unicode WCX API
;  2007-01-03 v. 1.02 - fixed handling command line arguments (just made buffers a bit longer)
;  2006-12-31 v. 1.01 - added support for SetProcessDataProc (some plugins expect callback to be set) - (thx to perchwasamara for hint on that)
;                     - fixed the way ProcessFile is called - some plugins I tested use the arguments provided, some don't
;                     - got amazed couple of times by misleading information provided in a WCX plug-in HELP file, but luckily had OllyDbg at hand :)
;  2006-12-30 v. 1.00 - first version
;
; -- Credits --
; - perchwasamara for hint on SetProcessDataProc
; - Jared B. for hint on problem with command line arguments
; - Hutch for his pieces of code I used in a proggie
; ;
; Feel free to play with the code to modify it up to your needs.
; If you happen to use my code, don't forget about creditz :-)
;
; Author: Konstantin Vlasov (c) 2013
; WWW:    http://flint-inc.ru/
; e-mail: support[]flint-inc[]ru
;
; Original work by:
; Author: Adam Blaszczyk (c) 2006/2007
; WWW:    http://www.kakeeware.com
; e-mail: adam[]kakeeware[]com
;
; ============================================================


.586
.MODEL FLAT, STDCALL
	assume fs:nothing

	include windows.inc

	NULL  = 0
	CR    = 0Dh
	LF    = 0Ah

	INV  equ INVOKE
	OFS  equ OFFSET
	BPTR equ BYTE PTR
	WPTR equ WORD PTR
	DPTR equ DWORD PTR

	MOM  MACRO t:REQ, s:REQ
		push   s
		pop    t
	ENDM

	INCLUDEX  MACRO plik:REQ
		include    plik.inc
		includelib plik.lib
	ENDM

	INCX MACRO mods:VARARG
		FOR c, <mods>
			INCLUDEX c
		ENDM
	ENDM

	INCX    kernel32, user32, masm32

	PK_OM_LIST    = 0
	PK_OM_EXTRACT = 1

	PK_SKIP       = 0
	PK_TEST       = 1
	PK_EXTRACT    = 2

	; OPENARCHIVEDATA and OPENARCHIVEDATAW differ only in what lies by ArcName and CmtBuf addresses,
	; but the structures themselves are identical; so we can use only one
	OPENARCHIVEDATAAW struct
		oadArcNameAW      dd ?
		oadOpenMode       dd ?
		oadOpenResult     dd ?
		oadCmtBufAW       dd ?
		oadCmtBufSize     dd ?
		oadCmtSize        dd ?
		oadCmtState       dd ?
	OPENARCHIVEDATAAW ends

	HEADERDATA struct
		hdArcName         db 260 dup(?)
		hdFileName        db 260 dup(?)
		hdFlags           dd ?
		hdPackSize        dd ?
		hdUnpSize         dd ?
		hdHostOS          dd ?
		hdFileCRC         dd ?
		hdFileTime        dd ?
		hdUnpVer          dd ?
		hdMethod          dd ?
		hdFileAttr        dd ?
		hdCmtBuf          dd ?
		hdCmtBufSize      dd ?
		hdCmtSize         dd ?
		hdCmtState        dd ?
	HEADERDATA ends

	HEADERDATAEXW struct
		hdArcNameW        dw 1024 dup(?)
		hdFileNameW       dw 1024 dup(?)
		hdFlags           dd ?
		hdPackSizeLow     dd ?
		hdPackSizeHigh    dd ?
		hdUnpSizeLow      dd ?
		hdUnpSizeHigh     dd ?
		hdHostOS          dd ?
		hdFileCRC         dd ?
		hdFileTime        dd ?
		hdUnpVer          dd ?
		hdMethod          dd ?
		hdFileAttr        dd ?
		hdCmtBuf          dd ?
		hdCmtBufSize      dd ?
		hdCmtSize         dd ?
		hdCmtState        dd ?
		hdReserved        db 1024 dup(?)
	HEADERDATAEXW ends

	; Unicode and ANSI interfaces will not be used simultaneously, so combine the structures together
	HEADERDATAAW union
		hdrA              HEADERDATA <>
		hdrW              HEADERDATAEXW <>
	HEADERDATAAW ends

.data?
	curDirName            db 1024 dup(?)
	curDirNameW           dw 1024 dup(?)
	argPluginNameW        dw 1024 dup(?)
	argOptionW            dw 1024 dup(?)
	argFilePathW          dw 1024 dup(?)
	argTargetDirW         dw 1024 dup(?)
	argFileNameW          dd ?

	hStdOut               dd ?
	hFile                 dd ?
	openMode              dd ?
	processMode           dd ?
	libHandle             dd ?

	OpenArchive           dd ?
	OpenArchiveW          dd ?
	ReadHeader            dd ?
	ReadHeaderExW         dd ?
	ProcessFile           dd ?
	ProcessFileW          dd ?
	CloseArchive          dd ?
	SetProcessDataProc    dd ?
	SetProcessDataProcW   dd ?

	archiveDataAW         OPENARCHIVEDATAAW <>
	archiveHandle         dd ?
	headerDataAW          HEADERDATAAW <>

	PreservedEip          dd ?
	PreservedEsp          dd ?
	PreservedEbp          dd ?

	tmpVar                dd ?
	tmpBuf                dw 1024 dup(?)    ; temporary buffer for ANSI or Unicode strings

.data
	useUnicode            db 1
	szInfo                db CR, LF
	                      db 'cmdTotal 2.02 (c) 2006/2007 KaKeeware, http://www.kakeeware.com', CR, LF
	                      db '              (c) 2013 Konstantin Vlasov, http://flint-inc.ru/', CR, LF
	                      db NULL
	szHelp                db CR, LF
	                      db 'Usage: cmdTotal [wcxPluginName] [option] [fileName] [targetDirectory]', CR, LF
	                      db '   where:', CR, LF
	                      db '     [wcxPluginName] - WCX plugin filename (must be in the same directory)', CR, LF
	                      db '     [option] - can be:', CR, LF
	                      db '       l - to list the files', CR, LF
	                      db '       t - to test the files', CR, LF
	                      db '       x - to extract the files', CR, LF
	                      db '     [fileName] - file you want to process', CR, LF
	                      db '     [targetDirectory] - where to extract the files (default: _[fileName])', CR, LF
	                      db 'Credits: perchwasamara, Jared B, Hutch', CR, LF
	                      db CR, LF, NULL
	szProcessing          db CR, LF, 'Using plugin:     %S', CR, LF
	                      db CR, LF, 'Using option:     %S', CR, LF
	                      db CR, LF, 'Processing file:  %S', CR, LF
	                      db CR, LF, 'Target directory: %S', CR, LF
	                      db NULL
	insideFile            db '    -> %s', CR, LF, NULL
	insideFileW           db '    -> %S', CR, LF, NULL
	backslashA            db '\', NULL
	backslashW            dw '\', NULL
	backslashUnderscoreW  dw '\', '_', NULL

	szOpenArchive         db 'OpenArchive', NULL
	szOpenArchiveW        db 'OpenArchiveW', NULL
	szReadHeader          db 'ReadHeader', NULL
	szReadHeaderExW       db 'ReadHeaderExW', NULL
	szProcessFile         db 'ProcessFile', NULL
	szProcessFileW        db 'ProcessFileW', NULL
	szCloseArchive        db 'CloseArchive', NULL
	szSetProcessDataProc  db 'SetProcessDataProc', NULL
	szSetProcessDataProcW db 'SetProcessDataProcW', NULL

	szGiveMeAFileName     db 'Give me some command line parameters!', CR, LF, NULL
	szCantLoadLib         db 'Can', 39, 't load the plug-in!', CR, LF, NULL
	szCantGetFunctions    db 'Can', 39, 't retrieve addresses to plug-in functions!', CR, LF, NULL
	szCrash               db 'Sorry, exception occured -> quitting!', CR, LF, NULL
	szGuard               db 'Sorry, program didn', 39, 't stop after 300 seconds -> quitting!', CR, LF, NULL
	szUnicodeErr          db 'Sorry, Unicode-ANSI conversion failed -> quitting!', CR, LF, NULL

.code
	; -----------------------------------------------------
	;  Copyright notice:
	; 'GetCLEx' function is almost an exact copy of GetCL function from masm32 package.
	; Differences are:
	; 1) size of the local buffers - the sizes have been expanded to 1024 bytes;
	; 2) using wide chars instead of ANSI.
	; -----------------------------------------------------
	GetCLEx proc ArgNum: DWORD, ItemBuffer: DWORD
		; -------------------------------------------------
		; arguments returned in "ItemBuffer"
		;
		; arg 0 = program name
		; arg 1 = 1st arg
		; arg 2 = 2nd arg etc....
		; -------------------------------------------------
		; Return values in eax
		;
		; 1 = successful operation
		; 2 = no argument exists at specified arg number
		; 3 = non matching quotation marks
		; 4 = empty quotation marks
		; -------------------------------------------------

		LOCAL lpCmdLineW:       DWORD
		LOCAL cmdBufferW[1024]: WORD
		LOCAL tmpBufferW[1024]: WORD

		push esi
		push edi

		invoke GetCommandLineW
		mov lpCmdLineW, eax     ; address of the command line

		; -------------------------------------------------
		; count quotation marks to see if pairs are matched
		; -------------------------------------------------
		xor ecx, ecx            ; zero ecx & use as counter
		mov esi, lpCmdLineW

		@@:
			lodsw
			cmp ax, 0
			je @F
			cmp ax, 34          ; [ " ] character
			jne @B
			inc ecx             ; increment counter
			jmp @B
		@@:

		push ecx                ; save count

		shr ecx, 1              ; integer divide ecx by 2
		shl ecx, 1              ; multiply ecx by 2 to get dividend

		pop eax                 ; put count in eax
		cmp eax, ecx            ; check if they are the same
		je @F
			pop edi
			pop esi
			mov eax, 3          ; return 3 in eax = non matching quotation marks
			ret
		@@:

		; ------------------------
		; replace tabs with spaces
		; ------------------------
		mov esi, lpCmdLineW
		lea edi, cmdBufferW

		@@:
			lodsw
			cmp ax, 0
			je rtOut
			cmp ax, 9           ; tab
			jne rtIn
			mov ax, 32
		rtIn:
			stosw
			jmp @B
		rtOut:
			stosw               ; write last word

		; -----------------------------------------------------------
		; substitute spaces in quoted text with replacement character
		; -----------------------------------------------------------
		lea eax, cmdBufferW
		mov esi, eax
		mov edi, eax

		subSt:
			lodsw
			cmp ax, 0
			jne @F
			jmp subOut
		@@:
			cmp ax, 34
			jne subNxt
			stosw
			jmp subSl           ; goto subloop
		subNxt:
			stosw
			jmp subSt

		subSl:
			lodsw
			cmp ax, 32          ; space
			jne @F
				mov ax, 254     ; substitute character
			@@:
			cmp ax, 34
			jne @F
				stosw
				jmp subSt
			@@:
			stosw
			jmp subSl

		subOut:
			stosw               ; write last word

		; ----------------------------------------------------
		; the following code determines the correct arg number
		; and writes the arg into the destination buffer
		; ----------------------------------------------------
		lea eax, cmdBufferW
		mov esi, eax
		lea edi, tmpBufferW

		mov ecx, 0              ; use ecx as counter

		; ---------------------------
		; strip leading spaces if any
		; ---------------------------
		@@:
			lodsw
			cmp ax, 32
			je @B

		l2St:
			cmp ecx, ArgNum     ; the number of the required cmdline arg
			je clSubLp2
			lodsw
			cmp ax, 0
			je cl2Out
			cmp ax, 32
			jne cl2Ovr          ; if not space

		@@:
			lodsw
			cmp ax, 32          ; catch consecutive spaces
			je @B

			inc ecx             ; increment arg count
			cmp ax, 0
			je cl2Out

		cl2Ovr:
			jmp l2St

		clSubLp2:
			stosw
		@@:
			lodsw
			cmp ax, 32
			je cl2Out
			cmp ax, 0
			je cl2Out
			stosw
			jmp @B

		cl2Out:
			mov ax, 0
			stosw

		; ------------------------------
		; exit if arg number not reached
		; ------------------------------
		.if ecx < ArgNum
			mov edi, ItemBuffer
			mov ax, 0
			stosw
			mov eax, 2          ; return value of 2 means arg did not exist
			pop edi
			pop esi
			ret
		.endif

		; -------------------------------------------------------------
		; remove quotation marks and replace the substitution character
		; -------------------------------------------------------------
		lea eax, tmpBufferW
		mov esi, eax
		mov edi, ItemBuffer

		rqStart:
			lodsw
			cmp ax, 0
			je rqOut
			cmp ax, 34          ; dont write [ " ] mark
			je rqStart
			cmp ax, 254
			jne @F
			mov ax, 32          ; substitute space
		@@:
			stosw
			jmp rqStart

		rqOut:
			stosw               ; write zero terminator

		; ------------------
		; handle empty quote
		; ------------------
		mov esi, ItemBuffer
		lodsw
		cmp ax, 0
		jne @F
			pop edi
			pop esi
			mov eax, 4          ; return value for empty quote
			ret
		@@:

		mov eax, 1              ; return value success

		pop edi
		pop esi

		ret

	GetCLEx endp

	; -----------------------------------------------------
	;  Copyright notice:
	; 'print' function is based on StdOut function from masm32 package
	; I rearranged it and removed pieces unnecessary in this proggie
	; -----------------------------------------------------
	print proc txt: DWORD
		LOCAL tmp: DWORD
		INV lstrlenA, txt
		xchg eax, edx
		INV WriteFile, hStdOut, txt, edx, ADDR tmp, NULL
		ret
	print endp

	fatalError proc
		INV print, OFS szCrash
		xor eax, eax
		inc eax                 ; eax=1
		ret
	fatalError endp

	guardThread proc lPa: DWORD
		INV Sleep, 300000       ; guard thread will close the program after 300 seconds
		INV print, OFS szGuard
		INV ExitProcess, -1
		ret
	guardThread endp

	sehit proc C exc, fra, con, dis: DWORD
		mov eax, con
		push PreservedEip
		pop  [eax + CONTEXT.regEip]

		push PreservedEsp
		pop  [eax + CONTEXT.regEsp]

		push PreservedEbp
		pop  [eax + CONTEXT.regEbp]

		xor eax, eax            ; ExceptionContinueExecution
		ret
	sehit endp

	; For now, there is no difference between ProcessDataProc and ProcessDataProcW,
	; but keep them apart in case future versions would process the parameters
	ProcessDataProc proc fil, siz: DWORD
		mov al, 1               ; return NON-ZERO
		ret
	ProcessDataProc endp

	ProcessDataProcW proc fil, siz: DWORD
		mov al, 1               ; return NON-ZERO
		ret
	ProcessDataProcW endp

Start:
	INV GetStdHandle, STD_OUTPUT_HANDLE
	mov hStdOut, eax

	INV SetUnhandledExceptionFilter, OFS fatalError

	INV CreateThread, NULL, NULL, OFS guardThread, NULL, NULL, NULL

	INV print, OFS szInfo

	INV GetCLEx, 1, OFS argPluginNameW
	cmp al, 1
	jne errBadCmdLine

	INV GetCLEx, 2, OFS argOptionW
	cmp al, 1
	jne errBadCmdLine

	mov ax, WPTR argOptionW
	or  ax, 20h
	.if ax == 'l'
		mov openMode, PK_OM_LIST
		mov processMode, PK_SKIP
	.elseif ax == 't'
		mov openMode, PK_OM_LIST
		mov processMode, PK_TEST
	.elseif ax == 'x'
		mov openMode, PK_OM_EXTRACT
		mov processMode, PK_EXTRACT
	.else
		jmp errBadCmdLine
	.endif

	INV GetCLEx, 3, OFS argFilePathW
	cmp al, 1
	jne errBadCmdLine

	.if openMode == PK_OM_EXTRACT
		INV GetCLEx, 4, OFS argTargetDirW
		.if eax == 1
			; create dir as requested
			INV lstrcpyW, OFS curDirNameW, OFS argTargetDirW
		.else
			; extract file name from argFilePathW
			; first, go to the end of string
			push esi
			mov esi, OFS argFilePathW

			; ecx will hold address of the last backslash, or address of the beginning of the string if no backslashes found
			mov ecx, esi
			@@:
				lodsw
				cmp ax, 0       ; reached end of the string?
				je  @F
				cmp ax, 5ch     ; backslash character?
				jne @B
				mov ecx, esi    ; store the address after backslash
				jmp @B
			@@:

			; now ecx contains the address of the file name
			mov argFileNameW, ecx
			pop esi

			; create underscoredDirectory directory
			INV GetCurrentDirectoryW, SIZEOF curDirNameW / 2, OFS curDirNameW
			INV lstrcatW, OFS curDirNameW, OFS backslashUnderscoreW
			INV lstrcatW, OFS curDirNameW, argFileNameW
			INV lstrcatW, OFS curDirNameW, OFS backslashW
		.endif
		INV CreateDirectoryW, OFS curDirNameW, 0
	.endif

	INV wsprintfA, OFS tmpBuf, OFS szProcessing, OFS argPluginNameW, OFS argOptionW, OFS argFilePathW, OFS argTargetDirW
	INV print, OFS tmpBuf

	; load Plug-in library
	INV LoadLibraryW, OFS argPluginNameW
	or  eax, eax
	je  errCantLoadLib

	mov libHandle, eax

	; get offsets to exported functions, try Unicode versions first
	INV GetProcAddress, libHandle, OFS szOpenArchiveW
	or  eax, eax
	je  noUnicode
	mov OpenArchiveW, eax

	INV GetProcAddress, libHandle, OFS szReadHeaderExW
	or  eax, eax
	je  noUnicode
	mov ReadHeaderExW, eax

	INV GetProcAddress, libHandle, OFS szProcessFileW
	or  eax, eax
	je  noUnicode
	mov ProcessFileW, eax

	; this function is not mandatory, so we don't care if it is exported
	; although we will try to call it to set our callback
	; - just in case the plug-in implements it and it assumes
	; it will be set to something <> NULL
	; (thx to perchwasamara for hint on that)
	INV GetProcAddress, libHandle, OFS szSetProcessDataProcW
	mov SetProcessDataProcW, eax

	jmp commonFunctions

noUnicode:
	; one or more required Unicode functions not found, fall back to ANSI
	mov  useUnicode, 0

	INV GetProcAddress, libHandle, OFS szOpenArchive
	or  eax, eax
	je  errCantGetFunctions
	mov OpenArchive, eax

	INV GetProcAddress, libHandle, OFS szReadHeader
	or  eax, eax
	je  errCantGetFunctions
	mov ReadHeader, eax

	INV GetProcAddress, libHandle, OFS szProcessFile
	or  eax, eax
	je  errCantGetFunctions
	mov ProcessFile, eax

	INV WideCharToMultiByte, CP_ACP, 0, OFS curDirNameW, -1, OFS curDirName, SIZEOF curDirName, NULL, NULL
	or  eax, eax
	je  errUnicode

commonFunctions:
	INV GetProcAddress, libHandle, OFS szCloseArchive
	or  eax, eax
	je  errCantGetFunctions
	mov CloseArchive, eax

	; TC provides both SetProcessDataProc and SetProcessDataProcW for Unicode plugins, and some plugins
	; (like ISO) use only ANSI version of this callback without even checking that it really was provided.
	INV GetProcAddress, libHandle, OFS szSetProcessDataProc
	mov SetProcessDataProc, eax

	; we are going to open the archive for extracting
	MOM archiveDataAW.oadOpenMode, openMode
	xor eax, eax
	mov archiveDataAW.oadCmtBufAW, eax
	mov archiveDataAW.oadCmtBufSize, eax
	mov archiveDataAW.oadCmtSize, eax
	mov archiveDataAW.oadCmtState, eax
	push OFS archiveDataAW
	.if useUnicode != 0
		mov archiveDataAW.oadArcNameAW, OFS argFilePathW
		call DPTR [OpenArchiveW]
	.else
		INV WideCharToMultiByte, CP_ACP, 0, OFS argFilePathW, -1, OFS tmpBuf, SIZEOF tmpBuf, NULL, NULL
		or  eax, eax
		je  errUnicode
		mov archiveDataAW.oadArcNameAW, OFS tmpBuf
		call DPTR [OpenArchive]
	.endif
	.if eax != 0
		mov archiveHandle, eax

		.if SetProcessDataProc != 0
			push ProcessDataProc
			push archiveHandle
			call DPTR [SetProcessDataProc]
		.endif
		.if useUnicode != 0
			.if SetProcessDataProcW != 0
				push ProcessDataProcW
				push archiveHandle
				call DPTR [SetProcessDataProcW]
			.endif
		.endif

nextHeader:
		push OFS headerDataAW
		push archiveHandle
		.if useUnicode != 0
			call DPTR [ReadHeaderExW]
		.else
			call DPTR [ReadHeader]
		.endif
		.if eax == 0
			.if useUnicode != 0
				INV wsprintfA, OFS tmpBuf, OFS insideFileW, OFS headerDataAW.hdrW.hdFileNameW
			.else
				INV wsprintfA, OFS tmpBuf, OFS insideFile, OFS headerDataAW.hdrA.hdFileName
			.endif
			INV print, OFS tmpBuf

			push OFS sehit      ; setup SEH frame to handle potential exceptions
			push fs:[0]
			mov  fs:[0], esp
			mov  PreservedEbp, ebp
			mov  PreservedEsp, esp
			mov  PreservedEip, OFS welcomeBack

			.if useUnicode != 0
				; Construct the full target path: TC passes DestPath == NULL and DestName == full path
				INV lstrcpyW, OFS tmpBuf, OFS curDirNameW
				INV lstrcatW, OFS tmpBuf, OFS backslashW
				INV lstrcatW, OFS tmpBuf, OFS headerDataAW.hdrW.hdFileNameW
				.if (headerDataAW.hdrW.hdFileAttr & 10h)
					; this element is a directory - create it and process as skipping
					INV CreateDirectoryW, OFS tmpBuf, 0
					; Prepare stack for ProcessFileW: TC passes DestPath and DestName as NULLs
					push NULL
					push NULL
					push PK_SKIP
				.else
					; this element is a file - prepare the stack for normal processing the file
					push OFS tmpBuf
					push OFS NULL
					push processMode
				.endif
				; archiveHandle is always passed
				push archiveHandle
				call DPTR [ProcessFileW]
			.else
				; Construct the full target path as ANSI
				INV lstrcpyA, OFS tmpBuf, OFS curDirName
				INV lstrcatA, OFS tmpBuf, OFS backslashA
				INV lstrcatA, OFS tmpBuf, OFS headerDataAW.hdrA.hdFileName
				.if (headerDataAW.hdrA.hdFileAttr & 10h)
					; this element is a directory - create it and process as skipping
					INV CreateDirectoryA, OFS tmpBuf, 0
					; Prepare stack for ProcessFile: TC passes DestPath and DestName as NULLs
					push NULL
					push NULL
					push PK_SKIP
				.else
					; this element is a file - prepare the stack for normal processing the file
					push OFS tmpBuf
					push OFS NULL
					push processMode
				.endif
				push archiveHandle
				call DPTR [ProcessFile]
			.endif

welcomeBack:
			pop fs:[0]          ; - restore SEH
			add esp, 4          ; /

			jmp nextHeader
		.endif                  ; if ReadHeader returned zero

		; close the archive
		push archiveHandle
		call DPTR [CloseArchive]
	.endif                      ; if OpenArchive returned non-zero

	INV FreeLibrary, libHandle

	jmp done

errCantGetFunctions:
	INV  print, OFS szCantGetFunctions
	jmp  done

errCantLoadLib:
	INV  print, OFS szCantLoadLib
	jmp  done

errBadCmdLine:
	INV print, OFS szHelp
	INV print, OFS szGiveMeAFileName
	jmp done

errUnicode:
	INV print, OFS szUnicodeErr
	jmp done

done:
	INV ExitProcess, 0

END Start
