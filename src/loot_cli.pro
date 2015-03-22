TARGET = lootcli
#CONFIG   -= qt

!include(../LocalPaths.pri) {
  message("paths to required libraries need to be set up in LocalPaths.pri")
}

SOURCES += main.cpp \
    lootthread.cpp

HEADERS  += \
    lootthread.h

CONFIG(debug, debug|release) {
  OUTDIR = $$OUT_PWD/debug
  DSTDIR = $$PWD/../../outputd
} else {
  OUTDIR = $$OUT_PWD/release
  DSTDIR = $$PWD/../../output
  QMAKE_CXXFLAGS += /Zi /GL
  QMAKE_LFLAGS += /DEBUG /LTCG /OPT:REF /OPT:ICF
}

OUTDIR ~= s,/,$$QMAKE_DIR_SEP,g
DSTDIR ~= s,/,$$QMAKE_DIR_SEP,g

QMAKE_CXXFLAGS -= -Zc:wchar_t-
QMAKE_CXXFLAGS += -Zc:wchar_t

QMAKE_LFLAGS += /ENTRY:"mainCRTStartup"

INCLUDEPATH += \
    "$${BOOSTPATH}" \
    "$${LOOTPATH}/include"
#    "$${LOOTPATH}/src" \
#    "$${LOOTPATH}/../../libloadorder/src" \
#    "$${LOOTPATH}/../../libespm" \
#    "$${LOOTPATH}/../../zlib" \
#    "$${LOOTPATH}/../../yaml-cpp/include"

#release:LIBS += \
#    -L"$${LOOTPATH}/build/Release" \
#    -L"$${LOOTPATH}/../../libloadorder/build/Release" \
#    -L"$${LOOTPATH}/../../yaml-cpp/build/Release" \
#    -L"$${LOOTPATH}/../../libgit2/build/Release" \

#debug:LIBS += \
#    -L"$${LOOTPATH}/build/Debug" \
#    -L"$${LOOTPATH}/../../libloadorder/build/Debug" \
#    -L"$${LOOTPATH}/../../yaml-cpp/build/Debug" \
#    -L"$${LOOTPATH}/../../libgit2/build/Debug" \

LIBS += \
    -L"$${ZLIBPATH}" \
    -L"$${ZLIBPATH}/build" \
    -L"$${BOOSTPATH}/stage/lib" \
    -L"$${LOOTPATH}" \
#    -lloot32 \
    -lversion -ladvapi32 -lshell32

## -llibyaml-cppmtd -lgit2 -lzlibstatic -lloadorder32 \

#QMAKE_PRE_LINK += $${LOOTPATH}/expdef -p -o $${LOOTPATH}/loot32.dll > $${LOOTPATH}/loot32.def $$escape_expand(\\n)
#QMAKE_PRE_LINK += lib /def:$${LOOTPATH}/loot32.def /out:$${LOOTPATH}/loot32.lib /machine:x86 $$escape_expand(\\n)

QMAKE_POST_LINK += xcopy /y /I $$quote($$OUTDIR\\lootcli*.exe) $$quote($$DSTDIR\\loot) $$escape_expand(\\n)

#DEFINES += LOOT_STATIC

OTHER_FILES += \
    SConscript
