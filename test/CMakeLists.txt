include_directories(include)

set(ME_LIB_TEST_SOURCE
        me_lib_boost_test.cpp
        matching/test_helper.cpp
        matching/validators/matching_validators_test.cpp
        matching/validators/new_order_request_validators_test.cpp
        matching/validators/validators_test.cpp
        matching/passive_order_book_test.cpp
        matching/matching_algo_cancel_test.cpp
        matching/matching_algo_insert_match_test.cpp
        matching/matching_algo_insert_response_test.cpp
        matching/matching_algo_min_exec_qty_validators_test.cpp
        matching/matching_algo_new_insert_validators_test.cpp
        matching/validators/cancel_request_validators_test.cpp
        engine/default_engine_event_handler_test.cpp
        engine/matching_engine_test.cpp)

add_executable(ME_LIB_TEST ${ME_LIB_TEST_SOURCE})

target_link_libraries(ME_LIB_TEST
        ME_LIB
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

add_test(ME_LIB_TEST ME_LIB_TEST)