cmake_minimum_required(VERSION 3.18)

project(rf-powermon-client)

add_library(rf-powermon-client rf_powermon_client.c)
target_include_directories(rf-powermon-client
  PUBLIC "."
)
