cmake_minimum_required(VERSION 3.21)
project(PS2Studio VERSION 0.18.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
qt_add_executable(PS2Studio
    src/main.cpp
    src/CodeEditor.h src/CodeEditor.cpp
    src/ClangdClient.h src/ClangdClient.cpp
    src/ProjectSearchDialog.h src/ProjectSearchDialog.cpp
    src/ProblemsWidget.h src/ProblemsWidget.cpp
    src/DebuggerWidget.h src/DebuggerWidget.cpp
    src/MainWindow.h src/MainWindow.cpp
    src/NewProjectDialog.h src/NewProjectDialog.cpp
    src/SceneDocument.h src/SceneDocument.cpp
    src/SceneView2D.h src/SceneView2D.cpp
    src/Viewport3D.h src/Viewport3D.cpp
    src/InspectorWidget.h src/InspectorWidget.cpp
    src/AssetDatabase.h src/AssetDatabase.cpp
    src/AssetCompiler.h src/AssetCompiler.cpp
    src/GameView.h src/GameView.cpp
    src/ProfilerWidget.h src/ProfilerWidget.cpp
    src/DiscDeployDialog.h src/DiscDeployDialog.cpp
    src/EnvironmentDoctorDialog.h src/EnvironmentDoctorDialog.cpp
    src/SetupWizardDialog.h src/SetupWizardDialog.cpp
    src/DependencyManagerDialog.h src/DependencyManagerDialog.cpp
    src/DocumentationDialog.h src/DocumentationDialog.cpp
    src/ValidationCenterDialog.h src/ValidationCenterDialog.cpp
    src/UpdateChecker.h src/UpdateChecker.cpp
    src/SupportBundle.h src/SupportBundle.cpp
    src/SampleProjectGenerator.h src/SampleProjectGenerator.cpp
    src/RuntimeTemplates.h src/RuntimeTemplates.cpp
    src/TyraTemplates.h src/TyraTemplates.cpp
)
target_link_libraries(PS2Studio PRIVATE Qt6::Widgets Qt6::Network)
if(WIN32)
  set_target_properties(PS2Studio PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS PS2Studio RUNTIME DESTINATION .)
install(DIRECTORY docs DESTINATION .)
install(DIRECTORY scripts DESTINATION .)
install(FILES BOOTSTRAP.cmd DESTINATION .)
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "PS2 Studio")
set(CPACK_PACKAGE_VENDOR "mjf_0.0")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "PS2 Studio")
set(CPACK_GENERATOR "ZIP")
if(WIN32)
  list(APPEND CPACK_GENERATOR "NSIS")
endif()
include(CPack)

add_custom_command(TARGET PS2Studio POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/scripts $<TARGET_FILE_DIR:PS2Studio>/scripts
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/BOOTSTRAP.cmd $<TARGET_FILE_DIR:PS2Studio>/BOOTSTRAP.cmd
)
