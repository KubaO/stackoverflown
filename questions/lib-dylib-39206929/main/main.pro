### main/main.pro
QT = widgets
CONFIG += c++11
TEMPLATE = app
SOURCES += main.cpp
LIBS += -L../lib1 -llib1
INCLUDEPATH += ..
DEPENDPATH += ..

defineReplace(libVersions) {
   # libVersions(1,2,3) - returns .1.2.3. .1.2. .1. .
   versions=.$${1}.$${2}.$${3}. .$${1}.$${2}. .$${1}. .
   return($$versions)
}
defineReplace(dylibs) {
   # dylibs(base,1,2,3) - returns libbase.1.2.3.dylib libbase.1.2.dylib ... libbase.dylib
   base = $$1
   versions = $$libVersions("$$2","$$3","$$4")
   libs =
   for (version, versions): libs += lib$${base}$${version}dylib
   return($$libs)
}
defineTest(deployLib) {
   # deployLib(target,target2path,target2,1,2,3)
   #   deploys target2path/libtarget2.1.2.3.dylib,... to the target's application bundle
   target = $$1
   libpath = $$2
   libtarget = $$3
   libs = $$dylibs($$libtarget,$$4,$$5,$$6)
   targetdir = $${target}.app/Contents/MacOS
   mktargetdir = "(test -d $$targetdir/ || mkdir -p $$targetdir/)"
   for (lib, libs) {
      out = $$targetdir/$$lib
      $${lib}.target = $$out
      $${lib}.commands = $$mktargetdir
      $${lib}.commands += "&& $$QMAKE_COPY_FILE $$libpath/$$libtarget/$$lib $$out"
      export($${lib}.target)
      export($${lib}.commands)
      QMAKE_EXTRA_TARGETS += $$lib
      PRE_TARGETDEPS += $$out
   }
   export(QMAKE_EXTRA_TARGETS)
   export(PRE_TARGETDEPS)
   return(true)
}

macx {
   deployLib(main, .., lib1, 1, 0, 0)
}
