@echo off
REM ------------------------------------------------------------
REM  posthub-init.bat
REM  Initialize posthub directory structure for Windows
REM  Creates:
REM     posthub/
REM       users.txt
REM       alice/maildir/{tmp,new,cur}
REM       bob/maildir/{tmp,new,cur}
REM ------------------------------------------------------------

set ROOT=posthub

echo Initializing posthub directory...

REM Create root
if not exist %ROOT% mkdir %ROOT%

REM Create users.txt with alice and bob
echo alice>%ROOT%\users.txt
echo bob>>%ROOT%\users.txt

REM Function-like block: create maildir for a given user
call :CREATEDIR alice
call :CREATEDIR bob

echo Done.
goto :EOF


:CREATEDIR
set USER=%~1
echo Creating maildir for %USER%...

mkdir %ROOT%\%USER% 2>nul
mkdir %ROOT%\%USER%\maildir 2>nul
mkdir %ROOT%\%USER%\maildir\tmp 2>nul
mkdir %ROOT%\%USER%\maildir\new 2>nul
mkdir %ROOT%\%USER%\maildir\cur 2>nul

goto :EOF
