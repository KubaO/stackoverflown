### sigslot-dll-39149263.pro
TEMPLATE = subdirs
SUBDIRS += lib1 lib2 main
main.depends += lib1 lib2
lib2.depends += lib1
