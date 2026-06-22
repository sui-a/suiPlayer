# 定义变量，保存所有需要查找的库的名称
set(libs jsoncpp spdlog ev amqpcpp pthread brpc leveldb dl
    gflags cpr elasticlient cpprest fdfsclient fastcommon 
    avcodec avformat gtest curl odb-mysql odb odb-boost 
    hiredis fmt redis++ etcd-cpp-api grpc++ 
    grpc  gpr protobuf 
    ssl crypto z
)

macro(find_libraries lib) 
    find_library(${lib}_LIBRARY ${lib} PATHS /usr/lib /usr/lib/x86_64-linux-gnu /usr/local/lib )
    if(${lib}_LIBRARY)
        set(${lib}_FOUND TRUE)
    else()
        message(WARNING "❌ 未找到依赖库: ${lib}")
    endif()
    
    if(NOT TARGET suiScaffold::${lib})
        add_library(suiScaffold::${lib} INTERFACE IMPORTED)
        set_target_properties(suiScaffold::${lib} PROPERTIES
            INTERFACE_LINK_LIBRARIES "${${lib}_LIBRARY}"
        )
    endif()
endmacro()

function(find_my_depends)
    foreach(lib ${libs})
        find_libraries(${lib})
        if(NOT ${lib}_FOUND)
            message(FATAL_ERROR "丢失关键依赖库: ${lib}")
        endif()
    endforeach()
endfunction()

function(add_my_depends)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${libs})
endfunction()
