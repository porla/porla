message("Checking ${CMAKE_SOURCE_DIR}/webui.zip")

if (EXISTS "${CMAKE_SOURCE_DIR}/webui.zip")
    message("Embedding ${CMAKE_SOURCE_DIR}/webui.zip in Porla")

    file(READ "${CMAKE_SOURCE_DIR}/webui.zip" content HEX)

    # Separate into individual bytes.
    string(REGEX MATCHALL "([A-Fa-f0-9][A-Fa-f0-9])" SEPARATED_HEX ${content})

    set(counter 0)
    foreach (hex IN LISTS SEPARATED_HEX)
        string(APPEND output_c "0x${hex},")
        MATH(EXPR counter "${counter}+1")
        if (counter GREATER 16)
            string(APPEND output_c "\n    ")
            set(counter 0)
        endif ()
    endforeach ()
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
