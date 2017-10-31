# Creates C resources file from files in given directory

set(INPUT_FILES
    ${GDAL_DATA_PATH}/gdal_datum.csv
    ${GDAL_DATA_PATH}/gcs.csv
    ${GDAL_DATA_PATH}/pcs.csv
    ${GDAL_DATA_PATH}/ellipsoid.csv
    ${GDAL_DATA_PATH}/coordinate_axis.csv
)


set(HEADER "${OUTPUT_FILE}.h")
set(SOURCE "${OUTPUT_FILE}.cpp")

file(WRITE ${HEADER} "#include <array>\n#include <cinttypes>\nnamespace infra::data { \n")
file(WRITE ${SOURCE} "#include <array>\n#include <cinttypes>\nnamespace infra::data { \n")

foreach(INPUT_FILE ${INPUT_FILES})
    message(STATUS "Generate embedded data ${INPUT_FILE}")

    get_filename_component(FILENAME ${INPUT_FILE} NAME_WE)

    # Replace filename spaces & extension separator for C compatibility
    string(MAKE_C_IDENTIFIER FILENAME ${FILENAME})
    file(READ ${INPUT_FILE} FILEDATA HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," FILEDATA ${FILEDATA})
    # Get the data LENGTH and add 1, so we can add a null terminator
    string(LENGTH ${FILEDATA} FILEDATA_LENGTH)
    math(EXPR FILEDATA_LENGTH "${FILEDATA_LENGTH}+1")
    file(APPEND ${HEADER} "extern const std::array<uint8_t, ${FILEDATA_LENGTH}> ${FILENAME};\n")
    file(APPEND ${SOURCE} "extern const std::array<uint8_t, ${FILEDATA_LENGTH}> ${FILENAME} = {{${FILEDATA}0x00}};\n")
endforeach()

file(APPEND ${HEADER} "}\n")
file(APPEND ${SOURCE} "}\n")
