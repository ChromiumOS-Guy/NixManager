include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(PLUGIN "NixManagerPlugin")

set(NIXMANAGER_PLUGIN_CPP_SOURCES
    nix-layer/nix-config.cpp
    nix-layer/backup-config.cpp
    libs/openprocess.cpp
    nix-layer/nix-interact.cpp
    nix-setup.cpp
    nix-layer/nixhub-api.cpp
    nix-layer/nix-wrapper.cpp
    worker-logic.cpp
    worker.cpp
    controller.cpp
    plugin.cpp
)

set(NIXMANAGER_PLUGIN_HEADERS
    nix-layer/nix-config.h
    nix-layer/backup-config.h
    libs/openprocess.h
    nix-layer/nix-interact.h
    nix-setup.h
    nix-layer/nixhub-api.h
    nix-layer/nix-wrapper.h
    worker-logic.h
    worker.h
    controller.h
    plugin.h
)

add_library(${PLUGIN} MODULE
    ${NIXMANAGER_PLUGIN_CPP_SOURCES}
    ${NIXMANAGER_PLUGIN_HEADERS}
)

set_target_properties(${PLUGIN} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PLUGIN})
qt5_use_modules(${PLUGIN} Qml Quick DBus)

target_link_libraries(
    ${PLUGIN} 
    Qt5::Qml 
    Qt5::Core
    Qt5::Network
)
target_compile_features(${PLUGIN} PRIVATE cxx_std_17)
string(REGEX MATCH "^[^-]+" ARCH_ONLY "${ARCH_TRIPLET}")
add_definitions(-DARCH="${ARCH_ONLY}") # arm64/x86_64 (assuming 64bit)

set(QT_IMPORTS_DIR "/lib/${ARCH_TRIPLET}")

install(TARGETS ${PLUGIN} DESTINATION ${QT_IMPORTS_DIR}/${PLUGIN}/)
install(FILES qmldir DESTINATION ${QT_IMPORTS_DIR}/${PLUGIN}/)