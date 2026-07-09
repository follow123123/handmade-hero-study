@echo off

set CommonComplierFlags=-MT -nologo -Gm- -GR- -EHa -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -wd4459 -wd4018 -DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 -FC -Z7  
set CommonLinkerFlags=-incremental:no user32.lib gdi32.lib winmm.lib

if not exist w:\build mkdir ..\..\build
pushd ..\..\build

REM 64-bit build
del *.pdb > NUL 2> NUL
set SAFE_TIME=%TIME: =0%
cl  %CommonComplierFlags% ..\handmade\code\handmade.cpp -Fmhandmade.map -LD /link -incremental:no -PDB:handmade_%date:~-4,4%%date:~-10,2%%date:~7,2%_%SAFE_TIME:~0,2%%SAFE_TIME:~3,2%%SAFE_TIME:~6,2%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender
cl  %CommonComplierFlags% ..\handmade\code\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
popd   
