cmake_minimum_required(VERSION 3.15)

    project(Pinball)
    
    add_executable(${PROJECT_NAME} "Pinball/main.cpp"   "Pinball/Physics.cpp" "Pinball/Renderer.cpp" "Pinball/ModelManager.cpp" "Pinball/Camera.cpp" "Pinball/ModelManager.hpp")
    
    
    find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)
    find_package(FreeGLUT REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(assimp CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
    unofficial::omniverse-physx-sdk::sdk

    $<IF:$<TARGET_EXISTS:FreeGLUT::freeglut>,FreeGLUT::freeglut,FreeGLUT::freeglut_static>

    assimp::assimp

    )



    