-include ../../Makefile.include

VERSION.TXT := $(CMAKE_SOURCE_DIR)/version.txt
APP_NAME=$(shell awk '/APP_NAME/ {print tolower($$2)}' $(VERSION.TXT))

ifeq ($(Configuration),)
  Configuration = Release
  ifeq ($(DEBUG_BUILD),yes)
    Configuration = Debug
  endif
endif

ifeq ($(OS),darwin_embedded)
  CMAKE_BUILD_ARGUMENTS = -G Xcode
else ifeq ($(GEN),Xcode)
  CMAKE_BUILD_ARGUMENTS = -G Xcode
else
  CMAKE_BUILD_ARGUMENTS = -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=$(Configuration)
endif

ifeq ($(BUILD_DIR),)
  BUILD_DIR=$(CMAKE_SOURCE_DIR)/build
endif

all:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/build/share
	cp $(PREFIX)/share/config.site $(BUILD_DIR)/build/share
	cd $(BUILD_DIR); $(CMAKE) $(CMAKE_BUILD_ARGUMENTS) \
          -DENABLE_TESTING=OFF -DENABLE_INTERNAL_FFMPEG=ON \
          -DENABLE_INTERNAL_RapidJSON=ON -DENABLE_INTERNAL_FMT=ON \
          -DENABLE_INTERNAL_FSTRCMP=ON -DENABLE_INTERNAL_FLATBUFFERS=ON \
          -DENABLE_INTERNAL_GTEST=OFF -DENABLE_INTERNAL_SPDLOG=ON \
          $(CMAKE_EXTRA_ARGUMENTS) $(CMAKE_SOURCE_DIR)

kodi:
	$(MAKE) -C $(BUILD_DIR)

install:
	$(MAKE) -C $(BUILD_DIR) install

clean:
	$(MAKE) -C $(BUILD_DIR) clean

distclean:
	cd $(BUILD_DIR) && rm -rf *
