TEMPLATE = subdirs

libproj.subdir = libtheframe

appproj.subdir = frame
appproj.depends = libproj

SUBDIRS = appproj \
    libproj
