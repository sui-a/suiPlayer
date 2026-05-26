# 定义变量，保存所有需要查找的库的名称
set(libs jsoncpp protobuf spdlog
    amqpcpp ev pthread brpc leveldb dl
    ssl crypto gflags cpr elasticlient
    cpprest etcd-cpp-api fdfsclient
    fastcommon avcodec avformat gtest
    curl odb-mysql odb odb-boost
    hiredis fmt
)

macro(find_libraries lib)
    find_library(${lib}_LIBRARY ${lib} PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib)
    if(${lib}_LIBRARY)
        set(${lib}_FOUND TRUE)
    endif()
    if(NOT TARGET suiScaffold::${lib})
        add_library(suiScaffold::${lib} INTERFACE IMPORTED)
        set_target_properties(suiScaffold::${lib} PROPERTIES
            INTERFACE_LINK_LIBRARIES "${${lib}_LIBRARY}"
        )
    else()
        message(STATUS "找到依赖库: " ${lib})
    endif()
endmacro()

function(find_my_depends)
    foreach(lib ${libs})
        find_libraries(${lib})
        if(NOT ${lib}_FOUND)
            message(FATAL_ERROR "丢失关键依赖库: ${lib}")
        endif()
    endforeach()
    target_link_libraries(${PROJECT_NAME} PUBLIC ${libs})
endfunction()