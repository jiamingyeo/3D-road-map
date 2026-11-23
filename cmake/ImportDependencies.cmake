include(FetchContent)

# Macro to import GLFW
macro(import_glfw)
    if(NOT TARGET glfw)  # Guard to prevent multiple inclusion
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        FetchContent_MakeAvailable(glfw)
    endif()
endmacro()

# Macro to import glm
macro(import_glm)
    if(NOT TARGET glm)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(glm)

        include_directories(${glm_SOURCE_DIR})
    endif()
endmacro()

# Macro to import glew
macro(import_glew)
    if(NOT TARGET glew_s)  # Guard to prevent multiple inclusion
        include(FetchContent)

        FetchContent_Declare(
            glew
            GIT_REPOSITORY https://github.com/omniavinco/glew-cmake.git
            GIT_TAG master  # Or use a specific commit or tag if preferred
        )

        # Disable shared builds if needed
        set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libs" FORCE)
        FetchContent_MakeAvailable(glew)

        # Optional: include directories if needed manually (though target_link_libraries handles it)
        include_directories(${glew_SOURCE_DIR}/include)
    endif()
endmacro()

# Macro to import EnTT
macro(import_entt)
    if(NOT TARGET EnTT)  # Guard to prevent multiple inclusion
        FetchContent_Declare(
            entt
            GIT_REPOSITORY https://github.com/skypjack/entt.git
            GIT_TAG v3.12.0  # Or the latest stable tag
        )
        FetchContent_MakeAvailable(entt)

        include_directories(${entt_SOURCE_DIR})
    endif()
endmacro()

# Macro to import SQLite3 (single-file amalgamation)
# Downloads the amalgamation zip from the official SQLite website and
# compiles sqlite3.c as a static library with warnings suppressed.
macro(import_sqlite3)
    if(NOT TARGET sqlite3)
        FetchContent_Declare(
            sqlite3_amalgamation
            URL https://www.sqlite.org/2024/sqlite-amalgamation-3450300.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(sqlite3_amalgamation)
 
        add_library(sqlite3 STATIC ${sqlite3_amalgamation_SOURCE_DIR}/sqlite3.c)
        target_include_directories(sqlite3 PUBLIC ${sqlite3_amalgamation_SOURCE_DIR})
 
        # Enable the RTree spatial index extension.
        # This MUST be defined or sqlite3_prepare_v2 will fail on any query
        # that references a virtual table created with USING rtree(...).
        # Python's system SQLite has this enabled by default; the amalgamation does not.
        target_compile_definitions(sqlite3 PUBLIC SQLITE_ENABLE_RTREE=1)
 
        # Suppress the many pedantic warnings inside the amalgamation
        if (MSVC)
            target_compile_options(sqlite3 PRIVATE /W0)
        else()
            target_compile_options(sqlite3 PRIVATE -w)
        endif()
    endif()
endmacro()


# cmake/ImportDependencies.cmake
macro(import_nlohmann_json)
    if(NOT TARGET nlohmann_json)
        FetchContent_Declare(nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        v3.11.3)
        set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
        set(JSON_Install    OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(nlohmann_json)
    endif()
endmacro()

# Macro to import all dependencies
macro(importDependencies)
    message(STATUS "Starting to import dependencies...")

    message(STATUS "Importing GLFW...")
    import_glfw()
    message(STATUS "GLFW imported successfully.")

    message(STATUS "Importing GLM...")
    import_glm()
    message(STATUS "GLM imported successfully.")

    message(STATUS "Importing GLEW...")
    import_glew()
    message(STATUS "GLEW imported successfully.")

    message(STATUS "Importing EnTT...")
    import_entt()
    message(STATUS "EnTT imported successfully.")

    message(STATUS "Importing sqlite3...")
    import_sqlite3()
    message(STATUS "sqlite3 imported successfully.")

    message(STATUS "Importing nlohmann_json...")
    import_nlohmann_json()
    message(STATUS "nlohmann_json imported successfully.")

    message(STATUS "All dependencies have been imported successfully.")
endmacro()