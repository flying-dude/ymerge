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
#pkg_check_modules(libalpm REQUIRED IMPORTED_TARGET libalpm)
pkg_check_modules(libsystemd REQUIRED IMPORTED_TARGET libsystemd)

include(GNUInstallDirs)

include_directories(src/libauracle)
include_directories(src/youtil)
include_directories(src/ymerge)
include_directories(src/libymerge)
include_directories(submodules/auracle/src)
include_directories(submodules/pacman/src)
include_directories(submodules/pacman/src/common)

# suppress warnings in libalpm and libpacman
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration -Wdeprecated-declarations")

# statically link libalpm
file(GLOB_RECURSE libalpm_FILES submodules/pacman/lib/libalpm/*)
add_library(libalpm STATIC ${libalpm_FILES})
target_link_libraries(libalpm
    LibArchive::LibArchive
)

# libalpm compile definitions
target_compile_definitions(libalpm PUBLIC
    LIB_VERSION="${YMERGE_VERSION}"
    SYSHOOKDIR="/usr/share/libalpm/hooks"

    # https://gitlab.archlinux.org/archlinux/packaging/packages/pacman/-/blob/main/PKGBUILD#L60
    SCRIPTLET_SHELL="/usr/bin/bash"
    LDCONFIG="/usr/bin/ldconfig"

    # should check for presence of header like so: https://gitlab.archlinux.org/pacman/pacman/-/blob/master/meson.build#L130
    HAVE_MNTENT_H="TRUE"
    HAVE_SYS_STATVFS_H="TRUE" FSSTATSTYPE=struct\ statvfs
    HAVE_SYS_PARAM_H="TRUE"
    HAVE_SYS_PARAM_H="TRUE"
    HAVE_SYS_MOUNT_H="TRUE"
    HAVE_SYS_TYPES_H="TRUE"

    HAVE_LIBSSL=1
    )

# create static lib containing pacman sources
file(GLOB_RECURSE libpacman_FILES submodules/pacman/src/pacman/* submodules/pacman/src/common/*)
list(REMOVE_ITEM libpacman_FILES ${CMAKE_CURRENT_SOURCE_DIR}/submodules/pacman/src/pacman/pacman-conf.c)
add_library(libpacman STATIC ${libpacman_FILES})

# pacman compile definitions
target_compile_definitions(libpacman PUBLIC
    CONFFILE="/etc/pacman.conf"
    DBPATH="/var/lib/pacman/"
    LOGFILE="/var/log/pacman.log"
    CACHEDIR="/var/cache/pacman/pkg/"
    HOOKDIR="/etc/pacman.d/hooks/"
    GPGDIR="/etc/pacman.d/gnupg/"
    ROOTDIR="/"
    PACKAGE_VERSION="${YMERGE_VERSION}"
    HAVE_STRNLEN="TRUE")

# pacman compile definitions
target_compile_definitions(libpacman PUBLIC
    CONFFILE="/etc/pacman.conf"
    DBPATH="/var/lib/pacman/"
    LOGFILE="/var/log/pacman.log"
    CACHEDIR="/var/cache/pacman/pkg/"
    HOOKDIR="/etc/pacman.d/hooks/"
    GPGDIR="/etc/pacman.d/gnupg/"
    ROOTDIR="/"
    PACKAGE_VERSION="${YMERGE_VERSION}"
    HAVE_STRNLEN="TRUE")

file(GLOB_RECURSE libymerge_FILES src/libymerge/* submodules/pacman/src/pacman/*)
list(REMOVE_ITEM libymerge_FILES ${CMAKE_CURRENT_SOURCE_DIR}/submodules/pacman/src/pacman/pacman-conf.c)
add_library(libymerge STATIC ${libymerge_FILES})
target_link_libraries(libymerge libalpm libpacman libauracle youtil)

# add pacman dependencies
find_package(LibArchive REQUIRED)
target_link_libraries(libymerge
    LibArchive::LibArchive # https://cmake.org/cmake/help/latest/module/FindLibArchive.html
)

file(GLOB_RECURSE ymerge_FILES src/ymerge/*)
add_executable(ymerge ${ymerge_FILES})
target_link_libraries(ymerge libymerge)

# https://stackoverflow.com/questions/35477940/linking-statically-openssl-crypto-library-in-cmake
# https://aur.archlinux.org/packages/openssl-static
#set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)
target_link_libraries(ymerge OpenSSL::Crypto)

file(GLOB_RECURSE youtil_FILES src/youtil/*)
add_library(youtil SHARED ${youtil_FILES})

#add_executable(auracle submodules/auracle/src/auracle_main.cc)
#target_link_libraries(auracle absl::container_common absl::strings libauracle libalpm)

file(GLOB_RECURSE libauracle_FILES src/libauracle/* submodules/auracle/src/auracle/* submodules/auracle/src/aur/*)
add_library(libauracle STATIC ${libauracle_FILES})
target_link_libraries(libauracle
    CURL::libcurl
    fmt::fmt
    nlohmann_json::nlohmann_json
    PkgConfig::libsystemd
    absl::flat_hash_set
    absl::status
    absl::time
)

#add_executable(yman src/yman/main.cpp)
#target_link_libraries(yman libymerge)

#install(TARGETS ymerge youtil auracle yman libymerge)
install(TARGETS ymerge youtil libymerge)

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
