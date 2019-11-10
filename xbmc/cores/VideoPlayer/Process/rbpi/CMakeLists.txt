if(CORE_PLATFORM_NAME_LC STREQUAL rbpi OR RASPBERRY_PI4_GBM)
  set(SOURCES ProcessInfoPi.cpp)

  set(HEADERS ProcessInfoPi.h)

  core_add_library(processPi)
endif()
