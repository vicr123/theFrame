cd build
mv frame/theFrame.app ./

LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH='libtheframe.framework/Versions/1/libtheframe'
QT_INSTALL_PLUGINS=$(/usr/local/opt/qt/bin/qmake -query QT_INSTALL_PLUGINS)
QT_HOST_LIBS=$(/usr/local/opt/qt/bin/qmake -query QT_HOST_LIBS)

# Install the render process
cp theframe-render/theframe-render theFrame.app/Contents/MacOS/
install_name_tool -change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore theFrame.app/Contents/MacOS/theframe-render
install_name_tool -change /usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui theFrame.app/Contents/MacOS/theframe-render
install_name_tool -change /usr/local/opt/qt/lib/QtConcurrent.framework/Versions/5/QtConcurrent @executable_path/../Frameworks/QtConcurrent.framework/Versions/5/QtConcurrent theFrame.app/Contents/MacOS/theframe-render

# Deploy Qt Offscreen
mkdir -p theFrame.app/Contents/PlugIns/platforms
cp $QT_INSTALL_PLUGINS/platforms/libqoffscreen.dylib theFrame.app/Contents/PlugIns/platforms
install_name_tool -change $QT_HOST_LIBS/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore theFrame.app/Contents/PlugIns/platforms/libqoffscreen.dylib
install_name_tool -change $QT_HOST_LIBS/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui theFrame.app/Contents/PlugIns/platforms/libqoffscreen.dylib

# Install the libtheframe framework
mkdir theFrame.app/Contents/Frameworks
cp -r libtheframe/libtheframe.framework theFrame.app/Contents/Frameworks
install_name_tool -change libthe-libs.1.dylib @executable_path/../Libraries/libthe-libs.1.dylib theFrame.app/Contents/Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH
install_name_tool -change /usr/local/opt/qt/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore theFrame.app/Contents/Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH
install_name_tool -change /usr/local/opt/qt/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui theFrame.app/Contents/Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH
install_name_tool -change /usr/local/opt/qt/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets theFrame.app/Contents/Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH
install_name_tool -change /usr/local/opt/qt/lib/QtSvg.framework/Versions/5/QtSvg @executable_path/../Frameworks/QtSvg.framework/Versions/5/QtSvg theFrame.app/Contents/Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH
install_name_tool -change /usr/local/opt/qt/lib/QtMacExtras.framework/Versions/5/QtMacExtras @executable_path/../Frameworks/QtMacExtras.framework/Versions/5/QtMacExtras theFrame.app/Contents/Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH
install_name_tool -change $LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH @executable_path/../Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH theFrame.app/Contents/MacOS/theFrame
install_name_tool -change $LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH @executable_path/../Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH theFrame.app/Contents/MacOS/theframe-render
