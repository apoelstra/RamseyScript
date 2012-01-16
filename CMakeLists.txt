CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(RamseyCalc)
#ADD_LIBRARY(MyLibrary STATIC libSource.c)

FIND_PACKAGE(PkgConfig)

ADD_DEFINITIONS(-Wall -W -Wextra -Werror --std=c99 -pedantic -O3)
FIND_PACKAGE(GTK2 COMPONENTS gtk)
PKG_CHECK_MODULES(GTHREAD REQUIRED gthread-2.0)
INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS})

IF(GTK2_FOUND)
  ADD_EXECUTABLE(ramsey main.c coloring.c filter.c permutation.c ramsey.c recurse.c sequence.c stream.c word.c
                 gtk-ui.c gtk-text-buffer-stream.c gtk-script-view.c gtk-script-run.c string-stream.c)
  TARGET_LINK_LIBRARIES(ramsey ${GTK2_LIBRARIES} ${GTHREAD_LIBRARIES})
ENDIF()

ADD_EXECUTABLE(ramsey-cli main-cli.c coloring.c filter.c permutation.c ramsey.c recurse.c sequence.c stream.c word.c)

