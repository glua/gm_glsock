dofile("../../common.lua")

RequireDefaultlibs()

SOLUTION        "glsock"

targetdir       "bin"
INCLUDES        "source_sdk"
INCLUDES        "gmod_sdk"
defines         {"NDEBUG"}

includedirs{"../../boost-master"}
libdirs{"../../boost-master/stage/lib"}

WINDOWS()
LINUX()

PROJECT()
SOURCE_SDK_LINKS()
configuration           "windows"
configuration           "linux"
  links{
    "boost_atomic",
    "boost_chrono",
    "boost_context",
    "boost_coroutine",
    "boost_date_time",
    "boost_filesystem",
    "boost_graph",
    "boost_locale",
    "boost_log_setup",
    "boost_log",
    "boost_math_c99f",
    "boost_math_c99l",
    "boost_math_c99",
    "boost_math_tr1f",
    "boost_math_tr1l",
    "boost_math_tr1",
    "boost_prg_exec_monitor",
    "boost_program_options",
    "boost_random",
    "boost_regex",
    "boost_serialization",
    "boost_signals",
    "boost_sync",
    "boost_system",
    "boost_thread",
    "boost_timer",
    "boost_unit_test_framework",
    "boost_wave",
    "boost_wserialization",
    "pthread",
  }
