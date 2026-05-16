function(add_sample sample_name source_file)
    add_executable(${sample_name} ${source_file})
    
    target_link_libraries(${sample_name}
        PRIVATE
            interpreter
    )
    
    target_compile_features(${sample_name} PRIVATE cxx_std_20)
    
    set_target_properties(${sample_name} PROPERTIES
        FOLDER "samples/${source_dir}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/samples/${source_dir}"
    )
endfunction()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
    add_sample(sample_interpreter main.cpp)
endif()

