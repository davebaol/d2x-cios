::d2x-beta.bat should be saved here in order for ModMii v4.6.0+ to use it: modules\More-cIOSs\{ANY FOLDER NAME}\d2x-beta.bat

::--------update what comes after "d2x-beta-rev=" and "ciosversion=" below----------

set ciosversion=${IOS_REV}
::must be <=65535

set d2x-beta-rev=${MAJOR_VER}-${MINOR_VER}
::This info is used to sign *00.app file for cIOS detection, everything after "=" but before the first "-" is considered the version # and it must be a number! Good examples are "5-beta1-test1" or "5" or "999". Bad examples are "5beta1" or "five-beta1" or "-5-beta1"
::everything after the first "-" is considered the version string (limited by 16 chars) and it only recognizes letters, numbers and + - _ ( ) [ ]

set cIOSFamilyName=d2x
::Only change this if building a cIOS that isn't a "d2x" branded cIOS. This string is limited by 16 chars and it only recognizes letters, numbers and + - _ ( ) [ ]


if "%wadname%"=="" goto:endofd2xbat

::------------------------Section to update (below)--------------------
::here you should enter the new beta cIOS hashes (under the OLD non-beta "wadname")
if /i "%wadname:~0,17%" EQU "cIOS249[37]-d2x-v" set md5=ab5a7f35774cb719bb5ce75ee6a78082
if /i "%wadname:~0,17%" EQU "cIOS250[37]-d2x-v" set md5=c5aa883a82b019481e891876eeda0168
if /i "%wadname:~0,17%" EQU "cIOS249[38]-d2x-v" set md5=79bf80e240737d766ecb6ea5fce7fff8
if /i "%wadname:~0,17%" EQU "cIOS250[38]-d2x-v" set md5=3462805a398872d02d80ea34828c55f0
if /i "%wadname:~0,17%" EQU "cIOS249[53]-d2x-v" set md5=e0a1fd1ba57ff5bd73bcdfd5616e4c96 
if /i "%wadname:~0,17%" EQU "cIOS250[53]-d2x-v" set md5=01c96492cbd94ca13daa4f316822ea31
if /i "%wadname:~0,17%" EQU "cIOS249[55]-d2x-v" set md5=4ad09fce709ae058b64e3be65cb11332
if /i "%wadname:~0,17%" EQU "cIOS250[55]-d2x-v" set md5=5f1b300060d4356c517b1b516273dca4
if /i "%wadname:~0,17%" EQU "cIOS249[56]-d2x-v" set md5=7ca2a3edfbbf5a63ad45c2ee0c2a3b73
if /i "%wadname:~0,17%" EQU "cIOS250[56]-d2x-v" set md5=0cb51ddbadddf8486de834ad125d0540
if /i "%wadname:~0,17%" EQU "cIOS249[57]-d2x-v" set md5=4ec8dff1afac6d451671187c63708fe1
if /i "%wadname:~0,17%" EQU "cIOS250[57]-d2x-v" set md5=ed90eb04c16901279824a4f1261f1641 
if /i "%wadname:~0,17%" EQU "cIOS249[58]-d2x-v" set md5=45372e6092787886d022607521da08ed
if /i "%wadname:~0,17%" EQU "cIOS250[58]-d2x-v" set md5=ff9b8266313f8130ebc31f26fdf623bb
::------------------------Section to update (above)--------------------

::----------Additional Instructions---------------
::save any of the following beta files to the following directory
::if any of the following is not found, the standard d2x module used in the previous ModMii release will continue to be used
::modules\More-cIOSs\{ANY FOLDER NAME}\mload.app
::modules\More-cIOSs\{ANY FOLDER NAME}\FAT.app
::modules\More-cIOSs\{ANY FOLDER NAME}\SDHC.app
::modules\More-cIOSs\{ANY FOLDER NAME}\EHCI.app
::modules\More-cIOSs\{ANY FOLDER NAME}\USBS.app
::modules\More-cIOSs\{ANY FOLDER NAME}\DIPP.app
::modules\More-cIOSs\{ANY FOLDER NAME}\ES.app
::modules\More-cIOSs\{ANY FOLDER NAME}\FFSP.app
::------------------------------------------------------

set md5alt=%md5%

::change some variables
set wadname=%wadname:~0,17%%d2x-beta-rev%
if /i "%name:~0,17%" NEQ "Advanced Download" set name=%wadname%

:endofd2xbat 

