find_package(PythonInterp)

set(CPPLINT_VERSION "1.6.1")

if(PYTHONINTERP_FOUND)
  set(CPPLINT_COMMAND "${PROJECT_SOURCE_DIR}/thirdparty/cpplint/cpplint.py" CACHE FILEPATH "CppLint command")
  set(CPPLINT_FOUND ON CACHE BOOL "CppLint found")
endif()

function(enable_check_style)
  if(CPPLINT_FOUND)
    add_custom_target(check_style)
  endif()
endfunction()

function(source_check_style TARGET TARGET_SOURCES)
  if(CPPLINT_FOUND)
    set(TARGET_NAME "check_style_${TARGET}")

    set(SOURCES_LIST)
    foreach(SOURCE_FILE ${TARGET_SOURCES})
        if(SOURCE_FILE MATCHES \\.c$|\\.cxx$|\\.cpp$|\\.cc$|\\.h$|\\.hh$)
            list(APPEND SOURCES_LIST ${SOURCE_FILE})
        endif()
    endforeach(SOURCE_FILE)

    add_custom_target(${TARGET_NAME} ${PYTHON_EXECUTABLE} ${CPPLINT_COMMAND} "--quiet" ${SOURCES_LIST}
                      DEPENDS ${SOURCES_LIST}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(check_style ${TARGET_NAME})
    set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "check_style")
  endif()
endfunction()

function(target_check_style TARGET)
  if(CPPLINT_FOUND)
    get_target_property(TARGET_SOURCES ${TARGET} SOURCES)

    source_check_style(${TARGET} "${TARGET_SOURCES}")
  endif()
endfunction()

enable_check_style()
