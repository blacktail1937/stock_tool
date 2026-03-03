# ============================================================
# Conan 自动集成脚本 - 支持 CMake Presets
# 功能：
#   - 检测是否在 Qt Creator 环境中运行
#   - 根据当前使用的 configurePreset 自动执行 conan install
#   - 支持多配置生成器（Ninja Multi-Config, Visual Studio）
# ============================================================

# 检测是否在 Qt Creator 环境中运行
function(is_qtcreator_env result)
    if(DEFINED ENV{QT_CREATOR} OR DEFINED QT_QMAKE_EXECUTABLE OR DEFINED Qt5_DIR OR DEFINED Qt6_DIR)
        set(${result} TRUE PARENT_SCOPE)
    else()
        set(${result} FALSE PARENT_SCOPE)
    endif()
endfunction()

# 获取当前使用的 configure preset 名称
function(get_current_preset_name result)
    # 默认值
    set(preset_name "default")
    
    # 尝试从环境变量获取
    if(DEFINED ENV{CMAKE_PRESET_NAME})
        set(preset_name "$ENV{CMAKE_PRESET_NAME}")
    elseif(DEFINED CMAKE_PRESET_NAME)
        set(preset_name "${CMAKE_PRESET_NAME}")
    elseif(DEFINED ENV{CMAKE_CONFIG_TYPE})  # 修复：DEFINENV -> DEFINED ENV
        set(preset_name "$ENV{CMAKE_CONFIG_TYPE}")
    endif()
    
    set(${result} "${preset_name}" PARENT_SCOPE)
endfunction()

# 根据 preset 获取构建类型
function(get_build_type_from_preset preset_name result)
    string(TOLOWER "${preset_name}" preset_lower)
    
    if(preset_lower MATCHES "debug")
        set(build_type "Debug")
    elseif(preset_lower MATCHES "release")
        set(build_type "Release")
    elseif(preset_lower MATCHES "relwithdebinfo")
        set(build_type "RelWithDebInfo")
    elseif(preset_lower MATCHES "minsizerel")
        set(build_type "MinSizeRel")
    else()
        # 检查 CMake 配置
        if(CMAKE_CONFIGURATION_TYPES)
            # 多配置生成器，默认使用第一个
            list(GET CMAKE_CONFIGURATION_TYPES 0 build_type)
        else()
            set(build_type "${CMAKE_BUILD_TYPE}")
            if(NOT build_type)
                set(build_type "Release")
            endif()
        endif()
    endif()
    
    set(${result} "${build_type}" PARENT_SCOPE)
endfunction()

# 根据 preset 获取架构信息
function(get_arch_from_preset preset_name result)
    string(TOLOWER "${preset_name}" preset_lower)
    
    if(preset_lower MATCHES "win32" OR preset_lower MATCHES "x86")
        set(arch "x86")
    elseif(preset_lower MATCHES "x64" OR preset_lower MATCHES "amd64")
        set(arch "x86_64")
    elseif(preset_lower MATCHES "arm64")
        set(arch "armv8")
    else()
        # 尝试从 CMake 获取
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(arch "x86_64")
        else()
            set(arch "x86")
        endif()
    endif()
    
    set(${result} "${arch}" PARENT_SCOPE)
endfunction()

# 查找 conan_toolchain.cmake 的可能位置
function(find_conan_toolchain result)
    set(TOOLCHAIN_PATHS
        "${CMAKE_BINARY_DIR}/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/build/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/conan-dependencies/build/conan/build/Debug/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/conan-dependencies/build/conan/build/Release/generators/conan_toolchain.cmake"
        "${CMAKE_BINARY_DIR}/conan-dependencies/build/generators/conan_toolchain.cmake"
    )
    
    set(FOUND_PATH "")
    foreach(path ${TOOLCHAIN_PATHS})
        if(EXISTS "${path}")
            set(FOUND_PATH "${path}")
            break()
        endif()
    endforeach()
    
    set(${result} "${FOUND_PATH}" PARENT_SCOPE)
endfunction()

# ============================================================
# 调试信息
# ============================================================
message(STATUS "cmake configuration type is ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
message(STATUS "CMake multi-config? ${CMAKE_CONFIGURATION_TYPES}")
message(STATUS "CMake build type (single-config only): ${CMAKE_BUILD_TYPE}")

# ============================================================
# 主逻辑
# ============================================================

# 获取当前 preset 信息
get_current_preset_name(CURRENT_PRESET)
get_build_type_from_preset("${CURRENT_PRESET}" BUILD_TYPE)
get_arch_from_preset("${CURRENT_PRESET}" ARCH)

message(STATUS "CMake Preset: ${CURRENT_PRESET}")
message(STATUS "Build Type: ${BUILD_TYPE}")
message(STATUS "Architecture: ${ARCH}")

# 查找已存在的 toolchain
find_conan_toolchain(CONAN_TOOLCHAIN_PATH)

if(CONAN_TOOLCHAIN_PATH)
    # 情况 1：找到已存在的 toolchain，直接使用
    message(STATUS "Found existing Conan toolchain: ${CONAN_TOOLCHAIN_PATH}")
    include("${CONAN_TOOLCHAIN_PATH}")
    
else()
    # 情况 2：未找到 toolchain
    message(STATUS "Conan toolchain not found")
    
    # 检测是否在 Qt Creator 环境中
    is_qtcreator_env(IN_QT_CREATOR)
    
    if(IN_QT_CREATOR)
        # Qt Creator 环境：只给提示，不自动执行
        message(STATUS "Qt Creator environment detected (Conan plugin should handle dependencies)")
        message(STATUS "If dependencies are missing, please:")
        message(STATUS "  1. Check that conanfile.txt/py exists in ${CMAKE_SOURCE_DIR}")
        message(STATUS "  2. Ensure Qt Creator's Conan plugin is enabled")
        message(STATUS "  3. Run 'Tools > Conan > Install' in Qt Creator")
        message(STATUS "  4. Or manually run:")
        message(STATUS "     cd ${CMAKE_BINARY_DIR}")
        message(STATUS "     conan install ${CMAKE_SOURCE_DIR} --build=missing -s build_type=${BUILD_TYPE}")
        
    else()
        # 非 Qt Creator 环境：自动执行 conan install
        message(STATUS "Running 'conan install' automatically for preset: ${CURRENT_PRESET}...")
        
        # 检查 conanfile 是否存在
        set(CONANFILE_FOUND FALSE)
        if(EXISTS "${CMAKE_SOURCE_DIR}/conanfile.txt")
            set(CONANFILE_PATH "${CMAKE_SOURCE_DIR}/conanfile.txt")
            set(CONANFILE_FOUND TRUE)
        elseif(EXISTS "${CMAKE_SOURCE_DIR}/conanfile.py")
            set(CONANFILE_PATH "${CMAKE_SOURCE_DIR}/conanfile.py")
            set(CONANFILE_FOUND TRUE)
        endif()
        
        if(NOT CONANFILE_FOUND)
            message(FATAL_ERROR "No conanfile.txt or conanfile.py found in ${CMAKE_SOURCE_DIR}")
        endif()
        
        # 构建 conan install 命令
        set(CONAN_INSTALL_CMD 
            conan install ${CMAKE_SOURCE_DIR} 
            # --output-folder=${CMAKE_BINARY_DIR}
            --build=missing
            -s build_type=${BUILD_TYPE}
        )
        
        # 根据编译器类型添加 settings
        if(MSVC)
            # 获取 MSVC 版本
            if(MSVC_VERSION GREATER_EQUAL 1930)
                set(COMPILER_VERSION "193")
            elseif(MSVC_VERSION GREATER_EQUAL 1920)
                set(COMPILER_VERSION "192")
            elseif(MSVC_VERSION GREATER_EQUAL 1910)
                set(COMPILER_VERSION "191")
            else()
                set(COMPILER_VERSION "190")
            endif()
            
            set(CONAN_INSTALL_CMD 
                ${CONAN_INSTALL_CMD}
                -s compiler=msvc
                -s compiler.version=${COMPILER_VERSION}
                -s compiler.runtime=dynamic
            )
        elseif(MINGW)
            set(CONAN_INSTALL_CMD 
                ${CONAN_INSTALL_CMD}
                -s compiler=gcc
                -s compiler.version=${CMAKE_CXX_COMPILER_VERSION}
                -s compiler.libcxx=libstdc++11
            )
        endif()
        
        # 如果有指定 CMake generator，传递给 Conan
        if(CMAKE_GENERATOR)
            set(CONAN_INSTALL_CMD 
                ${CONAN_INSTALL_CMD} 
                -c tools.cmake.cmaketoolchain:generator=${CMAKE_GENERATOR}
            )
        endif()
        
        message(STATUS "Executing: ${CONAN_INSTALL_CMD}")
        
        execute_process(
            COMMAND ${CONAN_INSTALL_CMD}
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
            RESULT_VARIABLE CONAN_RESULT
            ERROR_VARIABLE CONAN_ERROR
            OUTPUT_VARIABLE CONAN_OUTPUT
        )
        
        if(NOT CONAN_RESULT EQUAL 0)
            message(FATAL_ERROR "Conan install failed:\n${CONAN_ERROR}")
        else()
            message(STATUS "Conan install completed successfully")
        endif()
        
        # 安装后再次查找 toolchain
        find_conan_toolchain(CONAN_TOOLCHAIN_AFTER)
        
        if(CONAN_TOOLCHAIN_AFTER)
            message(STATUS "Using Conan toolchain: ${CONAN_TOOLCHAIN_AFTER}")
            include("${CONAN_TOOLCHAIN_AFTER}")
        else()
            message(FATAL_ERROR "Conan toolchain still not found after install!")
        endif()
    endif()
endif()
