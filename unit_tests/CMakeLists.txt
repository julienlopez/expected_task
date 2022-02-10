cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

set(EXE_TARGET_NAME UnitTests)

find_package(Catch2)

add_executable(
  ${EXE_TARGET_NAME}
  "main.cpp"
  "test_expectedtask.cpp"
  "test_chaining_returning_tasks.cpp"
  "test_traits.cpp"
  "test_void_return_management.cpp"
  "test_make_unexpected.cpp"
  "test_high_order_functions.cpp"
  "test_when_all.cpp"
  "test_operators.cpp")

target_link_libraries(${EXE_TARGET_NAME}
	PRIVATE
	expected_task::expected_task
	Catch2::Catch2
)

include(CTest)
include(Catch)
catch_discover_tests(${EXE_TARGET_NAME})
