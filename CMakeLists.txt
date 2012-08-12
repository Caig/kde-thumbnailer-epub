project(kde-thumbnailer-epub)
cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_path (EPUB_INCLUDE_DIR NAMES epub.h)
find_library (EPUB_LIBRARY NAMES epub libepub REQUIRED)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${QT_INCLUDES} ${KDE4_INCLUDES} ${EPUB_INCLUDE_DIR})

set(epubthumbnail_SRCS epubthumbnail.cpp)

kde4_add_plugin(epubthumbnail ${epubthumbnail_SRCS})
target_link_libraries(epubthumbnail ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${KDE4_KIO_LIBS} ${EPUB_LIBRARY})

install(TARGETS epubthumbnail DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES epubthumbnail.desktop DESTINATION ${SERVICES_INSTALL_DIR})