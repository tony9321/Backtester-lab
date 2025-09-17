# Institutional 5-year comprehensive backtesting system
add_executable(institutional_backtest institutional_backtest.cpp)
target_link_libraries(institutional_backtest quantlab_core)

# Strategy optimization framework
add_executable(strategy_optimizer strategy_optimizer.cpp)
target_link_libraries(strategy_optimizer quantlab_core)

message(STATUS "Apps directory ready")