project(kde-thumbnailer-epub)
cmake_minimum_required(VERSION 2.8.12)
set(QT_MIN_VERSION "5.2.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui XmlPatterns)
find_package(KF5 REQUIRED COMPONENTS KIO Archive)

set(epubthumbnail_SRCS epubthumbnail.cpp epub.cpp)

add_library(epubthumbnail MODULE ${epubthumbnail_SRCS})
target_link_libraries(epubthumbnail Qt5::Gui Qt5::XmlPatterns KF5::KIOWidgets KF5::Archive)

install(TARGETS epubthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES epubthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})
