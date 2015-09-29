kde-thumbnailer-epub
====================

A KDE thumbnail generator for the ePub file format.

http://kde-apps.org/content/show.php/KDE+ePub+Thumbnailer?content=151210

Installation from source
------------------------

    mkdir build
    cd build
    cmake -DKDE_INSTALL_USE_QT_SYS_PATHS=ON -DCMAKE_INSTALL_PREFIX=`kf5-config --prefix` ..
    make
    sudo make install
    kbuildsycoca5
