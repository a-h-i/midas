file(GLOB HEADER_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/include/exceptions/*.hpp")


add_library(exception INTERFACE ${HEADER_FILES})


