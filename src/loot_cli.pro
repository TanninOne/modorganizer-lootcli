TARGET = lootcli
CONFIG   -= qt

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
    "$(BOOSTPATH)" \
    "$(LOOTPATH)/src" \
    "$(LOOTPATH)/../libloadorder/src" \
    "$(LOOTPATH)/../libespm" \
    "$(LOOTPATH)/../zlib" \
    "$(LOOTPATH)/../yaml-cpp/include"

LIBS += \
    -L"$(LOOTPATH)/build/Release" \
    -L"$(LOOTPATH)/../libloadorder/build/Release" \
    -L"$(LOOTPATH)/../zlib/build" \
    -L"$(LOOTPATH)/../yaml-cpp/build/Release" \
    -L"$(LOOTPATH)/../libgit2/build/Release" \
    -L"$(BOOSTPATH)/stage/lib" \
    -lloot32 -llibyaml-cppmd -lgit2 -lzlibstatic -lloadorder32 \
    -lversion -ladvapi32 -lshell32

QMAKE_POST_LINK += xcopy /y /I $$quote($$OUTDIR\\lootcli*.exe) $$quote($$DSTDIR\\loot) $$escape_expand(\\n)

DEFINES += LOOT_STATIC
