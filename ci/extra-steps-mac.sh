cd build
mv frame/theFrame.app ./

ls
ls theFrame.app
ls theFrame.app/Contents
ls theFrame.app/Contents/MacOS

LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH='libtheframe.framework/Versions/1/libtheframe'

# Install the render process
cp theframe-render/theframe-render theFrame.app/Contents/MacOS/

# Install the libtheframe framework
mkdir theFrame.app/Contents/Frameworks
cp -r libtheframe/libtheframe.framework theFrame.app/Contents/Frameworks
install_name_tool -change $LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH @executable_path/../Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH theFrame.app/Contents/MacOS/theFrame
install_name_tool -change $LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH @executable_path/../Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH theFrame.app/Contents/MacOS/theframe-render
