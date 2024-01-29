# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.
file(GLOB_RECURSE SOURCES ./*.c ./control/*.c ./ui/*.c  ./images/*.c ./fonts/*.c)
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS . ./control ./ui
                       )
