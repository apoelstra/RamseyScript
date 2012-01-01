CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(RamseyCalc)
#ADD_LIBRARY(MyLibrary STATIC libSource.c)

ADD_DEFINITIONS(-Wall -W -Wextra -ansi -pedantic -g)
FIND_PACKAGE(GTK2 COMPONENTS gtk)
INCLUDE_DIRECTORIES(${GTK2_INCLUDE_DIRS})

IF(GTK2_FOUND)
  ADD_EXECUTABLE(ramsey main.c check.c coloring.c ramsey.c recurse.c sequence.c)
  TARGET_LINK_LIBRARIES(ramsey ${GTK2_LIBRARIES})
ENDIF()

