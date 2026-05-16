add_library(interpreter STATIC)

target_compile_features(interpreter PUBLIC cxx_std_20)

target_sources(interpreter
    PRIVATE
        # Lexer
        lexer/lexer.cpp
        
        # Parser
        parser/parser.cpp
        
        # AST
        ast/expression.cpp
        ast/statement.cpp
        
        
        # Semantic analysis
        semantics/symbol_table.cpp
        semantics/type_check.cpp
        
        # Runtime
        runtime/value.cpp
        runtime/environment.cpp
        runtime/interpreter.cpp

        # Core
        core/error_report.cpp 
        core/type.cpp
)

target_include_directories(interpreter
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)