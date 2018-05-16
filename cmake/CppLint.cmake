find_package(PythonInterp)

if(PYTHONINTERP_FOUND AND NOT CPPLINT_FOUND)
  file(DOWNLOAD "https://github.com/google/styleguide/archive/gh-pages.zip" "${CMAKE_CURRENT_BINARY_DIR}/styleguide.zip")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xz styleguide.zip
                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  set(CPPLINT_COMMAND "${CMAKE_CURRENT_BINARY_DIR}/styleguide-gh-pages/cpplint/cpplint.py" CACHE FILEPATH "CppLint commad")
  set(CPPLINT_FOUND ON CACHE BOOL "CppLint found")
endif()

function(enable_check_style)
  if(CPPLINT_FOUND)
    add_custom_target(check_style)
  endif()
endfunction()

function(target_check_style TARGET)
  if(CPPLINT_FOUND)
    get_target_property(TARGET_SOURCES ${TARGET} SOURCES)

    set(TARGET_NAME "check_style_${TARGET}")

    set(SOURCES_LIST)
    foreach(sourcefile ${TARGET_SOURCES})
        if(sourcefile MATCHES \\.c$|\\.cxx$|\\.cpp$|\\.cc$|\\.h$|\\.hh$)
            list(APPEND SOURCES_LIST ${sourcefile})
        endif()
    endforeach(sourcefile)

    add_custom_target(${TARGET_NAME} ${PYTHON_EXECUTABLE} ${CPPLINT_COMMAND} ${SOURCES_LIST}
                      DEPENDS ${SOURCES_LIST}
                      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    add_dependencies(check_style ${TARGET_NAME})
  endif()
endfunction()
