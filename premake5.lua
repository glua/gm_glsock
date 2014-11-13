dofile("../../common.lua")

RequireDefaultlibs()

SOLUTION        "glsock"

targetdir       "bin"
INCLUDES        "source_sdk"
INCLUDES        "gmod_sdk"
defines         {"NDEBUG"}

configuration "windows"
  includedirs{"../../boost/includes"}
  libdirs{"../../boost/libs"}

WINDOWS()
LINUX()

PROJECT()
SOURCE_SDK_LINKS()
configuration           "windows"
configuration           "linux"
