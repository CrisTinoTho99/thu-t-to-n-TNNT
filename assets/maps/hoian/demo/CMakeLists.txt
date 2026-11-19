add_executable(hoian_tour "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/Input.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/Log.cpp"
    "${CMAKE_SOURCE_DIR}/src/core/Window.cpp"
    "${CMAKE_SOURCE_DIR}/src/graphics/ShaderProgram.cpp")
target_include_directories(hoian_tour PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_compile_definitions(hoian_tour PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(hoian_tour PRIVATE glfw glad glm::glm vlt_world vlt_model_assets)
if(WIN32)
    target_link_libraries(hoian_tour PRIVATE opengl32)
endif()
