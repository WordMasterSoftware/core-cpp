include(ExternalProject)

set(WMNEXT_FLITE_PREFIX "${CMAKE_BINARY_DIR}/_deps/flite")
set(WMNEXT_FLITE_SOURCE_DIR "${WMNEXT_FLITE_PREFIX}/src/flite")
set(WMNEXT_FLITE_INSTALL_DIR "${WMNEXT_FLITE_PREFIX}/install")
set(WMNEXT_FLITE_INCLUDE_DIR "${WMNEXT_FLITE_INSTALL_DIR}/include")
set(WMNEXT_FLITE_INCLUDE_FLITE_DIR "${WMNEXT_FLITE_INSTALL_DIR}/include/flite")

ExternalProject_Add(wmnext_flite_project
    GIT_REPOSITORY https://github.com/festvox/flite.git
    GIT_TAG master
    GIT_SHALLOW TRUE
    PREFIX "${WMNEXT_FLITE_PREFIX}"
    SOURCE_DIR "${WMNEXT_FLITE_SOURCE_DIR}"
    CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env CFLAGS=-fPIC ./configure --prefix=${WMNEXT_FLITE_INSTALL_DIR} --with-audio=none
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
    INSTALL_COMMAND ${CMAKE_MAKE_PROGRAM} install
    BUILD_IN_SOURCE TRUE
    UPDATE_COMMAND ""
)

add_library(wmnext_flite_core STATIC IMPORTED GLOBAL)
set_target_properties(wmnext_flite_core PROPERTIES
    IMPORTED_LOCATION "${WMNEXT_FLITE_INSTALL_DIR}/lib/libflite.a"
    INTERFACE_INCLUDE_DIRECTORIES "${WMNEXT_FLITE_INCLUDE_DIR};${WMNEXT_FLITE_INCLUDE_FLITE_DIR}"
)
add_dependencies(wmnext_flite_core wmnext_flite_project)

add_library(wmnext_flite_usenglish STATIC IMPORTED GLOBAL)
set_target_properties(wmnext_flite_usenglish PROPERTIES
    IMPORTED_LOCATION "${WMNEXT_FLITE_INSTALL_DIR}/lib/libflite_usenglish.a"
    INTERFACE_INCLUDE_DIRECTORIES "${WMNEXT_FLITE_INCLUDE_DIR};${WMNEXT_FLITE_INCLUDE_FLITE_DIR}"
)
add_dependencies(wmnext_flite_usenglish wmnext_flite_project)

add_library(wmnext_flite_cmulex STATIC IMPORTED GLOBAL)
set_target_properties(wmnext_flite_cmulex PROPERTIES
    IMPORTED_LOCATION "${WMNEXT_FLITE_INSTALL_DIR}/lib/libflite_cmulex.a"
    INTERFACE_INCLUDE_DIRECTORIES "${WMNEXT_FLITE_INCLUDE_DIR};${WMNEXT_FLITE_INCLUDE_FLITE_DIR}"
)
add_dependencies(wmnext_flite_cmulex wmnext_flite_project)

add_library(wmnext_flite_voice_slt STATIC IMPORTED GLOBAL)
set_target_properties(wmnext_flite_voice_slt PROPERTIES
    IMPORTED_LOCATION "${WMNEXT_FLITE_INSTALL_DIR}/lib/libflite_cmu_us_slt.a"
    INTERFACE_INCLUDE_DIRECTORIES "${WMNEXT_FLITE_INCLUDE_DIR};${WMNEXT_FLITE_INCLUDE_FLITE_DIR}"
)
add_dependencies(wmnext_flite_voice_slt wmnext_flite_project)

add_library(wmnext_flite_voice_awb STATIC IMPORTED GLOBAL)
set_target_properties(wmnext_flite_voice_awb PROPERTIES
    IMPORTED_LOCATION "${WMNEXT_FLITE_INSTALL_DIR}/lib/libflite_cmu_us_awb.a"
    INTERFACE_INCLUDE_DIRECTORIES "${WMNEXT_FLITE_INCLUDE_DIR};${WMNEXT_FLITE_INCLUDE_FLITE_DIR}"
)
add_dependencies(wmnext_flite_voice_awb wmnext_flite_project)

add_library(wmnext_flite_voice_kal STATIC IMPORTED GLOBAL)
set_target_properties(wmnext_flite_voice_kal PROPERTIES
    IMPORTED_LOCATION "${WMNEXT_FLITE_INSTALL_DIR}/lib/libflite_cmu_us_kal.a"
    INTERFACE_INCLUDE_DIRECTORIES "${WMNEXT_FLITE_INCLUDE_DIR};${WMNEXT_FLITE_INCLUDE_FLITE_DIR}"
)
add_dependencies(wmnext_flite_voice_kal wmnext_flite_project)

add_library(wmnext_flite INTERFACE)
target_compile_definitions(wmnext_flite INTERFACE WMNEXT_WITH_FLITE=1)
target_include_directories(wmnext_flite INTERFACE "${WMNEXT_FLITE_INCLUDE_DIR}" "${WMNEXT_FLITE_INCLUDE_FLITE_DIR}")
target_link_libraries(wmnext_flite INTERFACE
    wmnext_flite_voice_slt
    wmnext_flite_voice_awb
    wmnext_flite_voice_kal
    wmnext_flite_usenglish
    wmnext_flite_cmulex
    wmnext_flite_core
    m
)
