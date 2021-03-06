set(IPCP_QOS_CUBE_BE_PRIO 50 CACHE STRING
  "Priority for best effort QoS cube (0-99)")
set(IPCP_QOS_CUBE_VIDEO_PRIO 90 CACHE STRING
  "Priority for video QoS cube (0-99)")
set(IPCP_QOS_CUBE_VOICE_PRIO 99 CACHE STRING
  "Priority for voice QoS cube (0-99)")
set(IPCP_MIN_THREADS 4 CACHE STRING
  "Minimum number of worker threads in the IPCP")
set(IPCP_ADD_THREADS 4 CACHE STRING
  "Number of extra threads to start when an IPCP faces thread starvation")
set(IPCP_SCHED_THR_MUL 2 CACHE STRING
  "Number of scheduler threads per QoS cube")
set(DISABLE_CORE_LOCK FALSE CACHE BOOL
  "Disable locking performance threads to a core")
set(IPCP_CONN_WAIT_DIR TRUE CACHE BOOL
  "Check the running state of the directory when adding a dt connection")
set(DHT_ENROLL_SLACK 50 CACHE STRING
  "DHT enrollment waiting time (0-999, ms)")

if ((IPCP_QOS_CUBE_BE_PRIO LESS 0) OR (IPCP_QOS_CUBE_BE_PRIO GREATER 99))
  message(FATAL_ERROR "Invalid priority for best effort QoS cube")
endif ()

if ((IPCP_QOS_CUBE_VIDEO_PRIO LESS 0) OR (IPCP_QOS_CUBE_VIDEO_PRIO GREATER 99))
  message(FATAL_ERROR "Invalid priority for video QoS cube")
endif ()

if ((IPCP_QOS_CUBE_VOICE_PRIO LESS 0) OR (IPCP_QOS_CUBE_VOICE_PRIO GREATER 99))
  message(FATAL_ERROR "Invalid priority for voice QoS cube")
endif ()

if ((DHT_ENROLL_SLACK LESS 0) OR (DHT_ENROLL_SLACK GREATER 999))
  message(FATAL_ERROR "Invalid DHT slack value")
endif ()


set(IPCP_SOURCES
  # Add source files here
  ${CMAKE_CURRENT_SOURCE_DIR}/ipcp.c
  ${CMAKE_CURRENT_SOURCE_DIR}/shim-data.c
  )

add_subdirectory(local)
add_subdirectory(normal)
add_subdirectory(raptor)
add_subdirectory(udp)
add_subdirectory(eth)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)
