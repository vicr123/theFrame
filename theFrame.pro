TEMPLATE = subdirs

libproj.subdir = libtheframe

appproj.subdir = frame
appproj.depends = libproj

renderproj.subdir = theframe-render
renderproj.depends = libproj

SUBDIRS = appproj \
    libproj \
    renderproj
