cmake_minimum_required(VERSION 3.19)

project(ymerge)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# use the same flags as makepkg so that we get the same warnings during development
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wp,-D_FORTIFY_SOURCE=2 -O -Wp,-D_GLIBCXX_ASSERTIONS")

if(DEFINED YMERGE_VERSION)
    add_definitions(-DYMERGE_VERSION="${YMERGE_VERSION}")
else()
    add_definitions(-DYMERGE_VERSION="UNKNOWN_VERSION")
endif()

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

include_directories(src/libauracle)
include_directories(src/youtil)
include_directories(src/ymerge)
include_directories(auracle/src)

file(GLOB_RECURSE ymerge_FILES src/ymerge/*)
add_executable(ymerge ${ymerge_FILES})
target_link_libraries(ymerge libauracle youtil)

file(GLOB_RECURSE youtil_FILES src/youtil/*)
add_library(youtil SHARED ${youtil_FILES})

add_executable(auracle auracle/src/auracle_main.cc)
target_link_libraries(auracle absl::container_common absl::strings libauracle)

file(GLOB_RECURSE libauracle_FILES src/libauracle/* auracle/src/auracle/* auracle/src/aur/*)
add_library(libauracle SHARED ${libauracle_FILES})
set_target_properties(libauracle PROPERTIES PREFIX "")
target_link_libraries(libauracle
    CURL::libcurl
    fmt::fmt
    nlohmann_json::nlohmann_json
    PkgConfig::libalpm
    PkgConfig::libsystemd
    absl::flat_hash_set
    absl::status
    absl::time
)

file(GLOB_RECURSE yman_FILES pacman/src/pacman/*)
list(REMOVE_ITEM yman_FILES ${CMAKE_CURRENT_SOURCE_DIR}/pacman/src/pacman/pacman-conf.c)
add_executable(yman ${yman_FILES})
find_package(LibArchive REQUIRED)
target_link_libraries(yman
    PkgConfig::libalpm
    LibArchive::LibArchive # https://cmake.org/cmake/help/latest/module/FindLibArchive.html
)

target_compile_definitions(yman PUBLIC
    CONFFILE="/etc/pacman.conf"
    DBPATH="/var/lib/pacman/"
    LOGFILE="/var/log/pacman.log"
    CACHEDIR="/var/cache/pacman/pkg/"
    HOOKDIR="/etc/pacman.d/hooks/"
    GPGDIR="/etc/pacman.d/gnupg/"
    ROOTDIR="/"
    PACKAGE_VERSION="${YMERGE_VERSION}"
    HAVE_STRNLEN="TRUE")


install(TARGETS ymerge youtil auracle libauracle yman)

install(FILES extra/bash_completion
    RENAME auracle
    DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions
)
install(FILES extra/zsh_completion
    RENAME auracle
    DESTINATION ${CMAKE_INSTALL_DATADIR}/zsh/site-functions
)

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
    add_manpage(MANFILE "ymerge"
        RELEASE "ymerge ${CMAKE_PROJECT_VERSION}"
        SECTION 1
        CENTER "ymerge Manual")
elseif(POD2MAN)
    message(WARNING "Failed to find pod2man, not generating manpage")
endif(POD2MAN)
