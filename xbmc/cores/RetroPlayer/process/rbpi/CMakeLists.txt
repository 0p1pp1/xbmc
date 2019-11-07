if(CORE_PLATFORM_NAME_LC STREQUAL rbpi OR RASPBERRY_PI4_GBM)
  set(SOURCES RPProcessInfoPi.cpp)

  set(HEADERS RPProcessInfoPi.h)

  core_add_library(rp-process-pi)
endif()
