file(READ ${INPUT_TEXTURE} TEXTURE_DATA HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," TEXTURE_DATA "${TEXTURE_DATA}")
string(LENGTH "${TEXTURE_DATA}" TEXTURE_LENGTH)
math(EXPR TEXTURE_SIZE "${TEXTURE_LENGTH} / 3")  # each byte is 3 chars (0xNN,)

set(CONTENT "#pragma once\n\n")
set(CONTENT "${CONTENT}#include <cstddef>\n\n")
set(CONTENT "${CONTENT}extern const unsigned char embedded_texture_data[];\n")
set(CONTENT "${CONTENT}extern const std::size_t embedded_texture_size;\n")

file(WRITE ${OUTPUT_HEADER} "${CONTENT}")

# Append the data in a .cpp file for definition
set(CPP_CONTENT "#include \"embedded_texture.h\"\n\n")
set(CPP_CONTENT "${CPP_CONTENT}const unsigned char embedded_texture_data[] = {${TEXTURE_DATA}};\n")
set(CPP_CONTENT "${CPP_CONTENT}const std::size_t embedded_texture_size = ${TEXTURE_SIZE};\n")

file(WRITE ${OUTPUT_CPP} "${CPP_CONTENT}")