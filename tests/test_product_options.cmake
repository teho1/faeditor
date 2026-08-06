include("${SOURCE_DIR}/cmake/Products.cmake")

faeditor_product_metadata(FA fa_target fa_name fa_bundle fa_version fa_build fa_uri fa_main)
if(NOT fa_target STREQUAL "FAEditor" OR NOT fa_name STREQUAL "FA Editor"
   OR NOT fa_bundle STREQUAL "com.righthere.faeditor" OR NOT fa_version STREQUAL "1.2"
   OR NOT fa_build STREQUAL "5" OR NOT fa_uri STREQUAL "FAEditor" OR NOT fa_main STREQUAL "Main")
    message(FATAL_ERROR "FA product metadata changed")
endif()

faeditor_product_metadata(FANTOM fantom_target fantom_name fantom_bundle fantom_version fantom_build fantom_uri fantom_main)
if(NOT fantom_target STREQUAL "FantomEditor" OR NOT fantom_name STREQUAL "Fantom Editor"
   OR NOT fantom_bundle STREQUAL "com.righthere.fantomeditor" OR NOT fantom_version STREQUAL "0.1"
   OR NOT fantom_build STREQUAL "1" OR NOT fantom_uri STREQUAL "FantomEditor" OR NOT fantom_main STREQUAL "FantomMain")
    message(FATAL_ERROR "FANTOM product metadata changed")
endif()
