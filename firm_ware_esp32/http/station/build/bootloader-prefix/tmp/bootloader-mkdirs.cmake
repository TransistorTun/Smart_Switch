# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/pham_tuan/esp/esp-idf/components/bootloader/subproject"
  "/home/pham_tuan/esp/esp-idf/my_project/smart_home_v1/firm_ware_esp32/http/station/build/bootloader"
  "/home/pham_tuan/esp/esp-idf/my_project/smart_home_v1/firm_ware_esp32/http/station/build/bootloader-prefix"
  "/home/pham_tuan/esp/esp-idf/my_project/smart_home_v1/firm_ware_esp32/http/station/build/bootloader-prefix/tmp"
  "/home/pham_tuan/esp/esp-idf/my_project/smart_home_v1/firm_ware_esp32/http/station/build/bootloader-prefix/src/bootloader-stamp"
  "/home/pham_tuan/esp/esp-idf/my_project/smart_home_v1/firm_ware_esp32/http/station/build/bootloader-prefix/src"
  "/home/pham_tuan/esp/esp-idf/my_project/smart_home_v1/firm_ware_esp32/http/station/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/pham_tuan/esp/esp-idf/my_project/smart_home_v1/firm_ware_esp32/http/station/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/pham_tuan/esp/esp-idf/my_project/smart_home_v1/firm_ware_esp32/http/station/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
