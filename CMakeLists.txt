CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(RamseyCalc)
#ADD_LIBRARY(MyLibrary STATIC libSource.c)

FIND_PACKAGE(PkgConfig)

ADD_DEFINITIONS(-Wall -W -Wextra -Werror --std=c99 -pedantic -g)

FILE(GLOB filters filter/*.c)
FILE(GLOB targets target/*.c)
FILE(GLOB ramseys ramsey/*.c)
FILE(GLOB dumps   dump/*.c)

ADD_EXECUTABLE(ramsey-cli main-cli.c file-stream.c ${filters} ${targets} ${dumps} ${ramseys} process.c recurse.c setting.c stream.c)

