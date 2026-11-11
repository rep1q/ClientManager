cmake_minimum_required(VERSION 3.16)
project(ClientManager VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/src/ui)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql )

set(PROJECT_SOURCES
    src/main.cpp
    # Database
    src/db/DatabaseManager.h
    src/db/DatabaseManager.cpp

    # Dialogs
    src/dialogs/Client.h
    src/dialogs/Client.cpp

    # Main window
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
    src/ui/MainWindow.ui

    # CSV
    src/csv/CsvExporter.h
    src/csv/CsvExporter.cpp
    src/csv/CsvImporter.h
    src/csv/CsvImporter.cpp

    # Workers
    src/workers/ExportWorker.h
    src/workers/ExportWorker.cpp
    src/workers/ImportWorker.h
    src/workers/ImportWorker.cpp
)

add_executable(ClientManager ${PROJECT_SOURCES})

target_include_directories(ClientManager PRIVATE
    src/db
    src/dialogs
    src/ui
    src/csv
    src/workers
)

target_link_libraries(ClientManager PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

set(SCHEMA_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/resources/db/schema.sql)
set(SCHEMA_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources/db)
file(MAKE_DIRECTORY ${SCHEMA_DESTINATION})
configure_file(
    ${SCHEMA_SOURCE}
    ${SCHEMA_DESTINATION}/schema.sql
    COPYONLY
)

if(WIN32)
    set_target_properties(ClientManager PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()