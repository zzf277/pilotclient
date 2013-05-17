TEMPLATE = lib
CONFIG += staticlib
CONFIG -= qt

DOXYFILE = Doxyfile.qmake

DOXY_INPUT = .
win32:!win32-g++ {
	doxy.commands = set DOXY_SRC_ROOT=$$PWD\\..\\src & \
			doxygen $$PWD\\$$DOXYFILE
}
else {
	doxy.commands = DOXY_SRC_ROOT="$$PWD/../src" \
			doxygen $$PWD/$$DOXYFILE
}
doxy.CONFIG = no_link target_predeps
doxy.depends = $$PWD/$$DOXYFILE
doxy.input = DOXY_INPUT
doxy.name = DOXY
doxy.output = html/index.html
QMAKE_EXTRA_COMPILERS += doxy