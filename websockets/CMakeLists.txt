set(SOURCE_FILES
        websockets.h websockets.cpp
        ../base64/base64.h ../base64/base64.cpp
        )

add_library(websockets ${SOURCE_FILES})