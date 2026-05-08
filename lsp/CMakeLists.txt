cmake_minimum_required(VERSION 3.26)
project(BibbleASM-lsp)

include(FetchContent)

FetchContent_Declare(BibbleASM
    GIT_REPOSITORY https://github.com/JannikBulow/BibbleASM
    GIT_TAG master
)
FetchContent_MakeAvailable(BibbleASM)


set(SOURCES
    src/main.cpp
    src/transport.cpp
)

set(HEADERS
    include/lsp/json.h
    include/lsp/transport.h
)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SOURCES} ${HEADERS})

add_executable(BibbleASM-lsp ${SOURCES} ${HEADERS})

target_include_directories(BibbleASM-lsp PUBLIC include)

target_compile_features(BibbleASM-lsp PUBLIC cxx_std_20)

target_link_libraries(BibbleASM-lsp PRIVATE BibbleASM-framework-static)