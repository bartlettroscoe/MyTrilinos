SET(HEADERS_IN_THIS_DIR "")

FILE(GLOB HEADERS_IN_THIS_DIR *.hpp)

FILE(GLOB SOURCES_IN_THIS_DIR *.cpp)

SET(HEADERS ${HEADERS} ${HEADERS_IN_THIS_DIR} PARENT_SCOPE)

SET( SOURCES ${SOURCES} ${SOURCES_IN_THIS_DIR} PARENT_SCOPE)
