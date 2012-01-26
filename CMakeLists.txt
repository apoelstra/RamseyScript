CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(RamseyCalc)
#ADD_LIBRARY(MyLibrary STATIC libSource.c)

FIND_PACKAGE(PkgConfig)

ADD_DEFINITIONS(-Wall -W -Wextra -Werror --std=c99 -pedantic -g)
FIND_PACKAGE(GTK2 COMPONENTS gtk)
PKG_CHECK_MODULES(GTHREAD REQUIRED gthread-2.0)
INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS})

FILE(GLOB filters filter/*.c)
FILE(GLOB targets target/*.c)
FILE(GLOB ramseys ramsey/*.c)
FILE(GLOB dumps   dump/*.c)

IF(GTK2_FOUND)
  ADD_EXECUTABLE(ramsey-gtk main.c file-stream.c ${filters} ${targets} ${dumps} ${ramseys} process.c recurse.c setting.c stream.c
                 gtk-ui.c gtk-text-buffer-stream.c gtk-script-view.c gtk-script-run.c string-stream.c)
  TARGET_LINK_LIBRARIES(ramsey-gtk ${GTK2_LIBRARIES} ${GTHREAD_LIBRARIES})
ENDIF()

ADD_EXECUTABLE(ramsey-cli main-cli.c file-stream.c ${filters} ${targets} ${dumps} ${ramseys} process.c recurse.c setting.c stream.c)

