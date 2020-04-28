cd build

LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH='libtheframe.framework/Versions/1/libtheframe'

# Install the render process
cp theframe-render/theframe-render frame/theFrame.app/Contents/MacOS/

# Install the libtheframe framework
mkdir frame/theFrame.app/Contents/Frameworks
cp -r libtheframe/libtheframe.framework frame/theFrame.app/Contents/Frameworks
install_name_tool -change $LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH @executable_path/../Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH theFrame.app/Contents/MacOS/theFrame
install_name_tool -change $LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH @executable_path/../Frameworks/$LIBTHEFRAME_FRAMEWORK_RELATIVE_PATH theFrame.app/Contents/MacOS/theframe-render
