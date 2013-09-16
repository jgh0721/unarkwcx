@echo off

if exist cmdTotal.obj del cmdTotal.obj
if exist cmdTotal.exe del cmdTotal.exe

c:\masm32\bin\ml /nologo /c /coff /Cp /Ic:\masm32\include cmdTotal.asm
if not exist cmdTotal.obj goto exit

c:\masm32\bin\Link /nologo /FILEALIGN:512  /SUBSYSTEM:CONSOLE /LIBPATH:c:\masm32\lib cmdTotal.obj
if not exist cmdTotal.exe goto exit

dir *.exe | find "cmdTotal.exe"

:exit
pause