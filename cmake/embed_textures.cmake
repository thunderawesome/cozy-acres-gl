# Expected variables from CMakeLists.txt: 
# INPUT_TEXTURES (List of paths)
# OUTPUT_HEADER
# OUTPUT_CPP

# Initialize contents
set(HEADER_CONTENT "#pragma once\n#include <cstddef>\n#include <map>\n#include <string>\n\n")
set(HEADER_CONTENT "${HEADER_CONTENT}struct EmbeddedTexture {\n    const unsigned char* data;\n    std::size_t size;\n};\n\n")
set(HEADER_CONTENT "${HEADER_CONTENT}extern const std::map<std::string, EmbeddedTexture> g_EmbeddedTextures;\n")

set(CPP_CONTENT "#include \"embedded_textures.h\"\n\n")
set(MAP_ENTRIES "")

# Loop through each texture path provided in the list
foreach(STR_PATH ${INPUT_TEXTURES})
    # Extract the filename (e.g., "grass" from "assets/textures/grass.png")
    get_filename_component(TEX_NAME ${STR_PATH} NAME_WE)
    
    # Read raw binary data as hex
    file(READ ${STR_PATH} HEX_DATA HEX)
    
    # Convert hex to C-style hex: 0xAA, 0xBB, ...
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," FORMATTED_DATA "${HEX_DATA}")
    
    # Calculate byte size
    string(LENGTH "${HEX_DATA}" HEX_LENGTH)
    math(EXPR ACTUAL_BYTE_COUNT "${HEX_LENGTH} / 2")

    # Add the raw byte array to the CPP file as a static (local) variable
    set(CPP_CONTENT "${CPP_CONTENT}static const unsigned char tex_data_${TEX_NAME}[] = {\n${FORMATTED_DATA}\n};\n\n")
    
    # Add a line for the map initialization later
    list(APPEND MAP_ENTRIES "    {\"${TEX_NAME}\", {tex_data_${TEX_NAME}, ${ACTUAL_BYTE_COUNT}}}")
endforeach()

# Build the global map initialization
string(REPLACE ";" ",\n" MAP_BODY "${MAP_ENTRIES}")
set(CPP_CONTENT "${CPP_CONTENT}const std::map<std::string, EmbeddedTexture> g_EmbeddedTextures = {\n${MAP_BODY}\n};\n")

# Write files to disk
file(WRITE ${OUTPUT_HEADER} "${HEADER_CONTENT}")
file(WRITE ${OUTPUT_CPP} "${CPP_CONTENT}")