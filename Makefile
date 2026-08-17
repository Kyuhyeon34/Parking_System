QT_BIN  ?= C:/msys64/ucrt64/bin
QMAKE   ?= $(QT_BIN)/qmake-qt5.exe
QT_MAKE ?= $(QT_BIN)/mingw32-make.exe

export PATH := $(QT_BIN);$(PATH)

# STM32_Programmer_CLI briefly touches the ST-Link's USB handle during the
# post-flash MCU reset. On native Windows consoles (PowerShell/cmd) that can
# race with make's own "Entering/Leaving directory" writes and produce a
# spurious "write error" even though the flash itself succeeded. Suppressing
# those directory messages removes the write that was racing.
MAKEFLAGS += --no-print-directory

.PHONY: all firmware qt flash run clean

all: firmware qt

firmware:
	$(MAKE) -C firmware

qt: qt-parking/build/Makefile
	"$(QT_MAKE)" -C qt-parking/build

qt-parking/build/Makefile: qt-parking/qt_parking.pro
	@powershell.exe -NoProfile -Command "New-Item -ItemType Directory -Force -Path 'qt-parking/build' | Out-Null"
	cd qt-parking/build && "$(QMAKE)" ../qt_parking.pro

flash:
	$(MAKE) -C firmware flash

# A relative path here (instead of $(CURDIR)) sidesteps a make quirk where
# CURDIR gets mangled for paths containing Korean characters.
run: all
	$(MAKE) flash
	powershell.exe -NoProfile -Command "Start-Process -FilePath 'qt-parking/build/release/qt_parking.exe'"

clean:
	$(MAKE) -C firmware clean
	$(RM) -r qt-parking/build
