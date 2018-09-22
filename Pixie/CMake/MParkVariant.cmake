# TODO(Ahura): It seems like ExternalProject has no option to not init the git
# submodules. Hence on a mac platform, googletest gets downloaded twice, once
# for as a submodule of the Pixie and again as a submodule of mpark_variant
include(ExternalProject)
ExternalProject_Add(MPark.Variant
        GIT_REPOSITORY      https://github.com/mpark/variant.git
        GIT_TAG             master
        SOURCE_DIR          "${CMAKE_BINARY_DIR}/mpark/variant"
        TIMEOUT             60
        GIT_PROGRESS        ON
        GIT_SHALLOW         ON
        GIT_SUBMODULES      ""
        BUILD_COMMAND       ""
        INSTALL_COMMAND     ""
        TEST_COMMAND        ""
        EXCLUDE_FROM_ALL    ON
        )
ExternalProject_Get_property(MPark.Variant SOURCE_DIR)
add_dependencies(Pixie MPark.Variant)

target_include_directories(Pixie PUBLIC ${SOURCE_DIR}/include)