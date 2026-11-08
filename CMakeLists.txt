cmake_minimum_required(VERSION 3.18)
project(DesktopCharacter)

# Compile overrides
option(USE_NEAREST_SCALEMODE "Force SDL_SCALEMODE_NEAREST for older SDL3 versions" FALSE)

# Compiler flags
option(RELEASE "Removes debug flags" TRUE)

# Remove static runpaths
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "")


add_executable(out)
target_sources(out
    PRIVATE
        src/main_runner.c
        src/anim.c
        src/image.c
        src/character.c
        src/config.c
)
if(WIN32)
    target_sources(out
        PRIVATE src/config_editor/preview_mem_win.c
    )
else()
    target_sources(out
        PRIVATE src/config_editor/preview_mem_linux.c
    )
endif()

target_include_directories(out
    PRIVATE
        thirdparty/include
)
target_link_directories(out
    PRIVATE
        thirdparty/lib
)
target_link_libraries(out
    PRIVATE
        SDL3
        SDL3_image
)
target_compile_options(out
    PRIVATE -Wall
)
target_link_options(out
    PRIVATE -Wl,-rpath,"$ORIGIN"
)

if(USE_NEAREST_SCALEMODE)
    target_compile_definitions(out
        PRIVATE USE_NEAREST_SCALEMODE
    )
endif()
if(RELEASE)
    target_compile_options(out
        PRIVATE -O2
    )
else()
    target_compile_options(out
        PRIVATE -g -fsanitize=address
    )
    target_link_options(out
        PRIVATE -fsanitize=address
    )
endif()


add_executable(editor)
target_sources(editor
    PRIVATE
        src/config_editor/main.c
        src/config_editor/raylib_draw.c
        src/config_editor/file_io.c
        src/config_editor/preview.c
        src/config.c
        src/image.c
        src/character.c
)
target_include_directories(editor
    PRIVATE
        thirdparty/include
)
target_link_directories(editor
    PRIVATE
        thirdparty/lib
)

target_link_libraries(editor
    PRIVATE
        SDL3
        SDL3_image
        raylib
        m
        pthread
)
if(WIN32)
    target_sources(editor
        PRIVATE src/config_editor/preview_mem_win.c
    )
    target_link_libraries(editor
        PRIVATE winmm gdi32
    )
else()
    target_sources(editor
        PRIVATE src/config_editor/preview_mem_linux.c
    )
endif()
target_compile_options(editor
    PRIVATE -Wall
)
target_link_options(editor
    PRIVATE -Wl,-rpath,"$ORIGIN"
)

if(USE_NEAREST_SCALEMODE)
    target_compile_definitions(editor
        PRIVATE USE_NEAREST_SCALEMODE
    )
endif()
if(RELEASE)
    target_compile_options(editor
        PRIVATE -O2
    )
else()
    target_compile_options(editor
        PRIVATE -g -fsanitize=address
    )
    target_link_options(editor
        PRIVATE -fsanitize=address
    )
endif()

install(
    TARGETS
        out editor
    DESTINATION
        cmake-build/
)
