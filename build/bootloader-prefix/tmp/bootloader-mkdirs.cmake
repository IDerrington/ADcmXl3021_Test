# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/IDerring/esp/esp-idf/components/bootloader/subproject"
  "C:/esp/ADcmXL3021/lcd/build/bootloader"
  "C:/esp/ADcmXL3021/lcd/build/bootloader-prefix"
  "C:/esp/ADcmXL3021/lcd/build/bootloader-prefix/tmp"
  "C:/esp/ADcmXL3021/lcd/build/bootloader-prefix/src/bootloader-stamp"
  "C:/esp/ADcmXL3021/lcd/build/bootloader-prefix/src"
  "C:/esp/ADcmXL3021/lcd/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/esp/ADcmXL3021/lcd/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/esp/ADcmXL3021/lcd/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
