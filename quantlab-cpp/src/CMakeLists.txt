# Core library - includes AlpacaClient and backtesting engine
add_library(quantlab_core STATIC
    data/alpaca_client.cpp
    backtest/backtest_engine.cpp
)

target_include_directories(quantlab_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(quantlab_core PUBLIC
    nlohmann_json::nlohmann_json
    fmt::fmt
    cpr::cpr
)