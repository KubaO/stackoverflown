### demand-load-39291032.pro
TEMPLATE = subdirs
SUBDIRS = lib1 lib1_demand main
main.depends = lib1_demand
lib1_demand.depends = lib1
