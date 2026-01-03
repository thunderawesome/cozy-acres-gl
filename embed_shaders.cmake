# converts shader files to C++ string literals

file(READ ${INPUT_VERT} VERT_SOURCE)
file(READ ${INPUT_FRAG} FRAG_SOURCE)

# Escape backslashes and quotes
string(REPLACE "\\" "\\\\" VERT_SOURCE "${VERT_SOURCE}")
string(REPLACE "\"" "\\\"" VERT_SOURCE "${VERT_SOURCE}")
string(REPLACE "\n" "\\n\"\n\"" VERT_SOURCE "${VERT_SOURCE}")

string(REPLACE "\\" "\\\\" FRAG_SOURCE "${FRAG_SOURCE}")
string(REPLACE "\"" "\\\"" FRAG_SOURCE "${FRAG_SOURCE}")
string(REPLACE "\n" "\\n\"\n\"" FRAG_SOURCE "${FRAG_SOURCE}")

set(CONTENT "#pragma once\n\n")
set(CONTENT "${CONTENT}const char* embedded_cube_vert = \"${VERT_SOURCE}\";\n\n")
set(CONTENT "${CONTENT}const char* embedded_cube_frag = \"${FRAG_SOURCE}\";\n")

file(WRITE ${OUTPUT_HEADER} "${CONTENT}")