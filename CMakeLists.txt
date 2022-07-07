cmake_minimum_required(VERSION 3.19)

project(auracle VERSION 0)
set(CMAKE_CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" ON)

option(ENABLE_SANITIZERS "Enable runtime sanitizing (for development)")
if (ENABLE_SANITIZERS)
    message("Enabling asan and ubsan")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()

find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt REQUIRED)
find_package(absl REQUIRED COMPONENTS status strings time format hash container_common)

find_package(PkgConfig REQUIRED)
pkg_check_modules(libalpm REQUIRED IMPORTED_TARGET libalpm)
pkg_check_modules(libsystemd REQUIRED IMPORTED_TARGET libsystemd)

include(GNUInstallDirs)

file(GLOB_RECURSE libauracle_FILES src/auracle/auracle/* src/auracle/aur/*)
add_library(libauracle SHARED ${libauracle_FILES})
set_target_properties(libauracle PROPERTIES PREFIX "")
target_link_libraries(libauracle
    CURL::libcurl
    fmt::fmt
    nlohmann_json::nlohmann_json
    PkgConfig::libalpm
    PkgConfig::libsystemd
    absl::status
    absl::time
    absl::flat_hash_set
)
target_include_directories(libauracle PUBLIC src/auracle/)
target_compile_definitions(libauracle PUBLIC -DPROJECT_VERSION=\"${CMAKE_PROJECT_VERSION}\")

add_executable(auracle src/auracle/auracle_main.cc)
target_link_libraries(auracle absl::container_common absl::strings libauracle)
install(TARGETS auracle)

install(FILES extra/bash_completion
    RENAME auracle
    DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
)
install(FILES extra/zsh_completion
    RENAME auracle
    DESTINATION
    ${CMAKE_INSTALL_DATADIR}/zsh/site-functions
)

#find_program(
#    CLANG_TIDY_EXE
#    NAMES "clang-tidy"
#    DOC "Path to clang-tidy executable"
#    )
#if(CLANG_TIDY_EXE)
#    set_target_properties(
#        auracle PROPERTIES
#        CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
#        )
#endif()

find_program(POD2MAN pod2man)
function(add_manpage)
    set(options)
    set(oneValueArgs SECTION RELEASE CENTER)
    set(multiValueArgs)
    cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(OUTFILE "${CMAKE_CURRENT_BINARY_DIR}/${ARGV1}.${arg_SECTION}")
    set(SOURCEFILE "${CMAKE_CURRENT_SOURCE_DIR}/man/${ARGV1}.${arg_SECTION}.pod")
    string(TOUPPER "${ARGV1}" NAME_UPCASE)

    add_custom_command(
        OUTPUT "${OUTFILE}"
        COMMAND "${POD2MAN}"
            --section=${arg_SECTION}
            --center=${arg_CENTER}
            --name=${NAME_UPCASE}
            --release=${arg_RELEASE}
            "${SOURCEFILE}"
            "${OUTFILE}"
        DEPENDS "${POD2MAN}" "${SOURCEFILE}"
    )

    install(FILES "${OUTFILE}" DESTINATION "${CMAKE_INSTALL_MANDIR}/man${arg_SECTION}")
    add_custom_target("manpage-${ARGV1}" ALL DEPENDS "${OUTFILE}")
endfunction()

if(POD2MAN)
    add_manpage(MANFILE "auracle"
        RELEASE "Auracle ${CMAKE_PROJECT_VERSION}"
        SECTION 1
        CENTER "Auracle Manual")
elseif(POD2MAN)
    message(WARNING "Failed to find pod2man, not generating manpage")
endif(POD2MAN)

if (BUILD_TESTS)
    find_package(GTest REQUIRED COMPONENTS gtest_main gmock CONFIG)
    include(GoogleTest)

    enable_testing()

    # Unit tests
    file(GLOB libauracle_test_FILES src/auracle/auracle_test/* src/auracle/aur_test/*)
    add_executable(libauracle_test ${libauracle_test_FILES})
    target_include_directories(libauracle PUBLIC src/auracle/auracle src/auracle/aur)
    target_link_libraries(libauracle_test GTest::gtest_main GTest::gmock libauracle)
    gtest_discover_tests(libauracle_test)

    # Integration tests
    include(FindPythonInterp)
    set(PYTHON_TESTS
        "buildorder"
        "clone"
        "custom_format"
        "info"
        "outdated"
        "raw_query"
        "regex_search"
        "search"
        "show"
        "sort"
        "update"
    )
    foreach(TEST ${PYTHON_TESTS})
        add_test(
            NAME ${TEST}
            COMMAND
                ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_${TEST}.py"
            WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH}
        )
    endforeach()
endif(BUILD_TESTS)
