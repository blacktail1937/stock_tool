cmake_minimum_required(VERSION 3.16)

# ===== 简化版版本号处理 =====
# if(DEFINED ENV{PROJECT_VERSION})
#     set(VERSION_STRING "$ENV{PROJECT_VERSION}")
# else()
#     # 尝试获取标签版本
#     execute_process(
#         COMMAND git describe --tags --abbrev=0
#         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#         OUTPUT_VARIABLE GIT_TAG
#         OUTPUT_STRIP_TRAILING_WHITESPACE
#         ERROR_QUIET
#     )

#     if(GIT_TAG MATCHES "^v?([0-9]+\\.[0-9]+\\.[0-9]+)")
#         set(VERSION_STRING "${CMAKE_MATCH_1}")
#     else()
#         set(VERSION_STRING "0.1.0")
#     endif()
# endif()

# project(stock_tool VERSION ${VERSION_STRING} LANGUAGES CXX)
project(stock_tool LANGUAGES CXX)

message(STATUS "Building version: ${PROJECT_VERSION}")

include(cmake/conan.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 REQUIRED)
find_package(SqliteOrm REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Quick Concurrent)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(${PROJECT_NAME}
    main.cpp
)

qt_add_qml_module(${PROJECT_NAME}
    URI stock_tool
    QML_FILES
        Main.qml
        dialogs/AddStockDialog.qml
        dialogs/SideWindow.qml
        dialogs/StockDetailWindow.qml
        dialogs/StockAddWindow.qml
        dialogs/StockImportWindow.qml
        component/DateTimePicker.qml
    # RESOURCES
    #     scripts/akshare_stock.py
    #     scripts/tushare_stock.py
    #     scripts/spider_stock.py
    SOURCES
        util/stockcalcuator.h
        python/pythonwrapper.h
        # python/pythonwrapper.cpp
        # stocksummarytablemodel.h
        # stocksummarytablemodel.cpp
        # astock/astockinfoloader.h
        # astock/astockinfoloader.cpp
        # database/databasemanager.cpp
        model/stocksummarytablemodel.h
        model/stocksummarytablemodel.cpp
        model/stockpurchaselistmodel.h
        model/stockpurchaselistmodel.cpp
        model/stocksalelistmodel.h
        model/stocksalelistmodel.cpp
        model/stockbaseinfolistmodel.h
        model/stockbaseinfolistmodel.cpp
        database/sqlitewrapper.h
        database/stocktable.h
        database/databasemanager.h
        database/dataloader.h
        database/dataloader.cpp
        util/scopeguard.h
        util/common.h
        util/logger.h

)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
# set_target_properties(${PROJECT_NAME} PROPERTIES
# #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${PROJECT_NAME}
#     MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#     MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#     MACOSX_BUNDLE TRUE
#     WIN32_EXECUTABLE TRUE
# )

# ===== 编译选项 =====
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        "/utf-8"
        "/Zc:inline"
        "/W4"
        "/WX"
    )
    target_compile_options(${PROJECT_NAME} PRIVATE "/wd4702")

    # if(DEFINED ENV{CI})
    #     target_compile_options(${PROJECT_NAME} PRIVATE "/analyze")
    # endif()
else()
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE Qt6::Quick Qt6::Qml Qt6::Core Qt6::Gui
    PRIVATE pybind11::embed pybind11::headers sqlite_orm::sqlite_orm
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/model
    ${CMAKE_SOURCE_DIR}/database
)


# before build
set(SCRIPT_SRC_DIR ${CMAKE_SOURCE_DIR}/scripts)  # 项目根目录的script
set(SCRIPT_DST_DIR ${CMAKE_BINARY_DIR}/scripts)  # build目录的script
set(EXE_SCRIPT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/scripts)
add_custom_target(
    copy_script_dir ALL
    # 步骤1：确保目标目录存在（避免拷贝失败）
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SCRIPT_DST_DIR}"
    # 步骤2：仅拷贝有变化的目录内容（Windows兼容）
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    ${SCRIPT_SRC_DIR}  # 源目录
    ${SCRIPT_DST_DIR}  # 目标目录
    COMMENT "Copying script directory to build directory: ${SCRIPT_DST_DIR}"
    VERBATIM
)
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${SCRIPT_DST_DIR}
    ${EXE_SCRIPT_DIR}
    COMMENT "Copying scripts to executable directory"
)
add_dependencies(${PROJECT_NAME} copy_script_dir)

# 安装规则
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ===== CPack 打包配置 =====
# set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
# set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
# set(CPACK_PACKAGE_VENDOR "blacktail")
# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Stock Analysis Tool")
# set(CPACK_GENERATOR "ZIP;NSIS")
# set(CPACK_SOURCE_IGNORE_FILES "/build/;/\\\\.git/;/\\\\.github/")
# set(CPACK_NSIS_MODIFY_PATH ON)

# include(CPack)

# ===== 测试（可选） =====
if(EXISTS ${CMAKE_SOURCE_DIR}/tests)
    enable_testing()
    add_subdirectory(tests)
endif()
