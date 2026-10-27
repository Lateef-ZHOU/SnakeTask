include(FetchContent)
if(NOT TARGET Lidar::HandleCore)
    FetchContent_Declare(lidar_handle_core
        GIT_REPOSITORY https://github.com/Lateef-ZHOU/HandleCore.git
        GIT_TAG 379401c85f0da11adde9a2f6fde8fe767fbae707)
    FetchContent_MakeAvailable(lidar_handle_core)
endif()
set(TASKCORE_WITH_QML ON)
if(NOT TARGET Lidar::TaskUI)
    FetchContent_Declare(lidar_taskcore
        GIT_REPOSITORY https://github.com/Lateef-ZHOU/TaskCore.git
        GIT_TAG 04b7279ba5d03396729f28aa9a717ff6785f7041)
    FetchContent_MakeAvailable(lidar_taskcore)
endif()
