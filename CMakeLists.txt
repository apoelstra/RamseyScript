CMAKE_MINIMUM_REQUIRED(VERSION 2.6.2)

PROJECT(RamseyCalc)

FIND_PACKAGE(PkgConfig)

ADD_DEFINITIONS(-Wall -W -Wextra -Werror --std=c99 -pedantic -O3)

FILE(GLOB filters filter/*.c)
FILE(GLOB targets target/*.c)
FILE(GLOB ramseys ramsey/*.c)
FILE(GLOB dumps   dump/*.c)

ADD_EXECUTABLE(ramsey-cli main-cli.c global.c file-stream.c ${filters} ${targets} ${dumps} ${ramseys} process.c recurse.c setting.c stream.c)
TARGET_LINK_LIBRARIES(ramsey-cli pthread)

