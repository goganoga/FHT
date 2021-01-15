add_executable(TestWebClient 
  "example/TestWebClient/TestWebClient.cpp"
)
add_dependencies(TestWebClient
  Boost
  fht
)
target_link_libraries(TestWebClient 
  ${FHT_LIBRARIES}
)

add_executable(TestTaskHendler 
  "example/TestTaskHendler/TestTaskHendler.cpp"
)
add_dependencies(TestTaskHendler
  Boost
  fht
)
target_link_libraries(TestTaskHendler 
  ${FHT_LIBRARIES}
)

add_executable(TestWebServer 
  "example/TestWebServer/TestWebServer.cpp" 
  "example/TestWebServer/Test/Test.cpp"
)
add_dependencies(TestWebServer 
  Boost
  fht
)
target_link_libraries(TestWebServer 
  ${FHT_LIBRARIES}
)
add_custom_command(
        TARGET TestWebServer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/example/TestWebServer/config.json
                ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/config.json
)

add_executable(TestPostgresConnect
  "example/TestPostgresConnect/TestPostgresConnect.cpp" 
)
add_dependencies(TestPostgresConnect 
  Boost
  fht
)
target_link_libraries(TestPostgresConnect 
  fht
  ${FHT_LIBRARIES}
)
add_custom_command(
        TARGET TestPostgresConnect POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/example/TestPostgresConnect/configSQL.json
                ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_OUTPUT_PATH}/${BUILD_TYPE}/configSQL.json
)