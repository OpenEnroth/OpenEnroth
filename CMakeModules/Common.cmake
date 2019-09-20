
function(DEBUG_PRINT msg)
    if (NOT (CMAKE_BUILD_TYPE MATCHES "RELEASE"))
        message(STATUS ${msg})
    endif()
endfunction()


# This function works around a CMake issue with the Ninja generator where it
# does not understand imported libraries, and instead needs `BUILD_BYPRODUCTS`
# explicitly set.
# https://cmake.org/pipermail/cmake/2015-April/060234.html
function(NINJA_WORKAROUND_GET_BYPRODUCTS TARGET)
    string(TOUPPER ${TARGET} NAME)
    if (CMAKE_GENERATOR MATCHES "Ninja")
        get_target_property(BYPRODUCTS ${TARGET} IMPORTED_LOCATION)

        get_target_property(ADDITIONAL_BYPRODUCTS ${TARGET} INTERFACE_LINK_LIBRARIES)
        if(NOT ("${ADDITIONAL_BYPRODUCTS}" STREQUAL "") AND NOT ("${ADDITIONAL_BYPRODUCTS}" STREQUAL "ADDITIONAL_BYPRODUCTS-NOTFOUND"))
            LIST(APPEND BYPRODUCTS ${ADDITIONAL_BYPRODUCTS})
        endif()

        DEBUG_PRINT("${NAME}_BYPRODUCTS         ${BYPRODUCTS}")
        set(${NAME}_BYPRODUCTS ${BYPRODUCTS} PARENT_SCOPE)
    else()
        # Make this function a no-op when not using Ninja.
        set(${NAME}_BYPRODUCTS "" PARENT_SCOPE)
    endif()
endfunction()
