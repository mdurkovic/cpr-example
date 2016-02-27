set(SRCS ${SRCS}
    "${CMAKE_CURRENT_SOURCE_DIR}/Cache.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CacheUtils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Request.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/CachedResponse.cpp"
    PARENT_SCOPE)

set(HDRS ${HDRS}
    "${CMAKE_CURRENT_SOURCE_DIR}/Cache.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CacheUtils.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Request.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/CachedResponse.h"
    PARENT_SCOPE)
