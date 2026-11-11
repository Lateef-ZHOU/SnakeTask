include(FetchContent)
if(NOT TARGET Lidar::HandleCore)
    FetchContent_Declare(lidar_handle_core
        GIT_REPOSITORY https://github.com/Lateef-ZHOU/HandleCore.git
        GIT_TAG 7ad9b380fe8d58cdb00a97107e06a96f6f4ba9b0)
    FetchContent_MakeAvailable(lidar_handle_core)
endif()
set(TASKCORE_WITH_QML ON)
if(NOT TARGET Lidar::TaskUI)
    FetchContent_Declare(lidar_taskcore
        GIT_REPOSITORY https://github.com/Lateef-ZHOU/TaskCore.git
        GIT_TAG 8ba73dda2080f5d59a5ca710b31dcb3c8f479aed)
    FetchContent_MakeAvailable(lidar_taskcore)
endif()
