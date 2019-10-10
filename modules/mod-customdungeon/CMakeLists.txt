
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/mod_customdungeon.cpp")

AC_ADD_SCRIPT_LOADER("mod_customdungeon" "${CMAKE_CURRENT_LIST_DIR}/src/loader_customdungeon.h")


CU_ADD_HOOK(AFTER_WORLDSERVER_CMAKE "${CMAKE_CURRENT_LIST_DIR}/src/cmake/after_ws_install.cmake")
