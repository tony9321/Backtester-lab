# Institutional 5-year comprehensive backtesting system
add_executable(institutional_backtest institutional_backtest.cpp)
target_link_libraries(institutional_backtest quantlab_core)

message(STATUS "Apps directory ready")