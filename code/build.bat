@echo off

set CommonComplierFlags=-MT -nologo -Gm- -GR- -EHa -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4459 -wd4018 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7 -Fmwin32_handmade.map 
set CommonLinkerFlags=user32.lib gdi32.lib winmm.lib

if not exist w:\build mkdir ..\..\build
pushd ..\..\build

REM 64-bit build
cl  %CommonComplierFlags% ..\handmade\code\win32_handmade.cpp /link %CommonLinkerFlags%
popd   
