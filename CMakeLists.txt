cmake_minimum_required(VERSION 3.10)
project(CapsHotkey)

set(CMAKE_CXX_STANDARD 20)

#set(WIN32GUI_DIR ../win32-gui-wrapper/code/)
#set(WIN32GUI_SRC 
#    ${WIN32GUI_DIR}/src/thApplication.cpp
#    ${WIN32GUI_DIR}/src/thButton.cpp
#    ${WIN32GUI_DIR}/src/thCheckBox.cpp
#    ${WIN32GUI_DIR}/src/thComboBox.cpp
#    ${WIN32GUI_DIR}/src/thComboBoxItems.cpp
#    ${WIN32GUI_DIR}/src/thCommonDialog.cpp
#    ${WIN32GUI_DIR}/src/thEditBox.cpp
#    ${WIN32GUI_DIR}/src/thFile.cpp
#    ${WIN32GUI_DIR}/src/thFont.cpp
#    ${WIN32GUI_DIR}/src/thForm.cpp
#    ${WIN32GUI_DIR}/src/thGroupBox.cpp
#    ${WIN32GUI_DIR}/src/thHeight.cpp
#    ${WIN32GUI_DIR}/src/thImageList.cpp
#    ${WIN32GUI_DIR}/src/thLabel.cpp
#    ${WIN32GUI_DIR}/src/thListBox.cpp
#    ${WIN32GUI_DIR}/src/thListBoxItems.cpp
#    ${WIN32GUI_DIR}/src/thListView.cpp
#    ${WIN32GUI_DIR}/src/thListViewColumn.cpp
#    ${WIN32GUI_DIR}/src/thListViewItem.cpp
#    ${WIN32GUI_DIR}/src/thLogger.cpp
#    ${WIN32GUI_DIR}/src/thMDIChild.cpp
#    ${WIN32GUI_DIR}/src/thMDIClient.cpp
#    ${WIN32GUI_DIR}/src/thMemo.cpp
#    ${WIN32GUI_DIR}/src/thMenu.cpp
#    ${WIN32GUI_DIR}/src/thMenuCommon.cpp
#    ${WIN32GUI_DIR}/src/thObject.cpp
#    ${WIN32GUI_DIR}/src/thPopupMenu.cpp
#    ${WIN32GUI_DIR}/src/thPosX.cpp
#    ${WIN32GUI_DIR}/src/thPosY.cpp
#    ${WIN32GUI_DIR}/src/thRadioButton.cpp
#    ${WIN32GUI_DIR}/src/thString.cpp
#    ${WIN32GUI_DIR}/src/thText.cpp
#    ${WIN32GUI_DIR}/src/thToolbar.cpp
#    ${WIN32GUI_DIR}/src/thWidth.cpp
#    ${WIN32GUI_DIR}/src/thWin32App.cpp
#    ${WIN32GUI_DIR}/src/thWin32Logger.cpp
#    ${WIN32GUI_DIR}/src/thWindow.cpp
#)

find_package(imgui CONFIG REQUIRED)

aux_source_directory(. SRCS)
add_executable(${PROJECT_NAME} WIN32 ${SRCS} res/app.rc)
target_include_directories(${PROJECT_NAME} PRIVATE $ENV{SIMPLE_CPP})
target_compile_definitions(${PROJECT_NAME} PRIVATE _UNICODE UNICODE)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)
