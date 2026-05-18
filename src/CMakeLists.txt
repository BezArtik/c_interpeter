add_library(interpreter STATIC)

target_compile_features(interpreter PUBLIC cxx_std_20)

target_sources(interpreter
    PRIVATE
        lexer/lexer.cpp
        
        parser/parser.cpp
        
        core/error_report.cpp 
        core/type.cpp
        core/token.cpp

        ast/expression.cpp
        ast/statement.cpp
        
        semantics/symbol_table.cpp
        semantics/type_check.cpp
        
        runtime/value.cpp
        runtime/environment.cpp
        runtime/interpreter.cpp
)

target_include_directories(interpreter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(cint
    main.cpp
 "core/token.cpp")

target_link_libraries(cint PRIVATE interpreter)