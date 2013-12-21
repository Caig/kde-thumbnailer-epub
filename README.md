kde-thumbnailer-epub
====================

A KDE thumbnail generator for the ePub file format.

http://kde-apps.org/content/show.php/KDE+ePub+Thumbnailer?content=151210

Installation from source
------------------------

  mkdir build
  cd build
  cmake -DCMAKE_INSTALL_PREFIX=`kde4-config --prefix` ..
  make
  sudo make install
  kbuildsycoca4
