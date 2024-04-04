message("Checking ${CMAKE_CURRENT_LIST_DIR}/html/webui.zip")

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/html/webui.zip")
    message("Embedding ${CMAKE_CURRENT_LIST_DIR}/html/webui.zip in Porla")

    execute_process(
        COMMAND /bin/sh -c "hexdump -ve '1/1 \"0x%.2x,\"' ${CMAKE_CURRENT_LIST_DIR}/html/webui.zip"
        OUTPUT_VARIABLE output_c)
endif()

set(WEBUI_SRC "
#include \"stddef.h\"
#include \"stdint.h\"

const uint8_t data[] = {
    ${output_c}
};

extern \"C\" const uint8_t* webui_zip_data()
{
    return data;
}

extern \"C\" const size_t webui_zip_size()
{
    return sizeof(data);
}")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/webui.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/webui.cpp WEBUI_SRC_)
else()
    set(WEBUI_SRC_ "")
endif()

if (NOT "${WEBUI_SRC}" STREQUAL "${WEBUI_SRC_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/webui.cpp "${WEBUI_SRC}")
endif()
