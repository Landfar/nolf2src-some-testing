@echo off
rem windows-coop.bat -- local co-op on one PC (docs/MODERN.md M-G93), modern
rem build only: two games at once, each player's in a window of its own.
rem
rem   Player 1, keyboard and mouse: this folder's game (+LocalCoop 1), as it
rem   is set up (play it in a borderless window, Display's "Window mode").
rem   Host a co-op game from it (Multiplayer, Host).
rem
rem   Player 2, the controller: a second game (+LocalCoop 2) in a borderless
rem   window on another monitor (P2_MONITOR), which never takes the focus
rem   and reads only the controller. Join player 1's game from it
rem   (Multiplayer, Join, LAN). Its profile is "Player 2" (+CoopProfile
rem   <name> for another). Its sound is silent while player 1's window has
rem   the focus.
rem
rem From this folder (P2_DIR empty), player 2's settings are files of its
rem own, coop2.cfg (a copy of autoexec.cfg to start with) and
rem coop2-display.cfg. P2_DIR names another install of the game instead,
rem whose own settings player 2 then uses.
setlocal
cd /d "%~dp0"

rem Player 2's monitor: -1 the first one that is not the primary one, or
rem 1, 2, ... counted from the left; 0 the primary one
set P2_MONITOR=-1
rem Player 2's install of the game, if not this one (a full path)
set P2_DIR=

rem Both games go straight to the main menu, without the intro movies
rem (NoMovies; for this start only)
set P2_ARGS=+LocalCoop 2 +NoMovies 1 +Windowed 1 +WindowMode 2 +WindowMonitor %P2_MONITOR%

start "" "%~dp0NOLF2.exe" +LocalCoop 1 +NoMovies 1

rem NOLF2.exe writes the engine's command line to launchcmds.txt in its
rem folder: the first game reads it before the second writes it
timeout /t 5 /nobreak >nul

if "%P2_DIR%"=="" (
    if not exist coop2.cfg copy /y autoexec.cfg coop2.cfg >nul
    start "" "%~dp0NOLF2.exe" -config coop2.cfg -display coop2-display.cfg -windowtitle Player2 %P2_ARGS%
) else (
    start "" /D "%P2_DIR%" "%P2_DIR%\NOLF2.exe" -windowtitle Player2 %P2_ARGS%
)
