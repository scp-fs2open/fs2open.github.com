@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by FRED.HPJ. >"hlp\fred.hm"
echo. >>"hlp\fred.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\fred.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\fred.hm"
echo. >>"hlp\fred.hm"
echo // Prompts (IDP_*) >>"hlp\fred.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\fred.hm"
echo. >>"hlp\fred.hm"
echo // Resources (IDR_*) >>"hlp\fred.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\fred.hm"
echo. >>"hlp\fred.hm"
echo // Dialogs (IDD_*) >>"hlp\fred.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\fred.hm"
echo. >>"hlp\fred.hm"
echo // Frame Controls (IDW_*) >>"hlp\fred.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\fred.hm"
REM -- Make help for Project FRED


echo Building Win32 Help files
start /wait hcrtf -x "hlp\fred.hpj"
echo.
if exist Debug\nul copy "hlp\fred.hlp" Debug
if exist Debug\nul copy "hlp\fred.cnt" Debug
if exist Release\nul copy "hlp\fred.hlp" Release
if exist Release\nul copy "hlp\fred.cnt" Release
echo.


