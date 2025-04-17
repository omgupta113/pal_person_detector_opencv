<<<<<<< HEAD
cmake_minimum_required(VERSION 2.8.3)
project(pal_person_detector_opencv)

find_package(catkin REQUIRED COMPONENTS image_transport
                                        pal_detection_msgs
                                        cv_bridge
                                        roscpp)
find_package(OpenCV REQUIRED)


catkin_package(
  CATKIN_DEPENDS image_transport cv_bridge pal_detection_msgs roscpp
)

include_directories(SYSTEM ${catkin_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

add_executable(pal_person_detector_opencv src/person_detector.cpp)
target_link_libraries(pal_person_detector_opencv ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

install(TARGETS pal_person_detector_opencv
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})
=======
cmake_minimum_required(VERSION 3.8)
project(pal_person_detector_opencv)

# Default to C++14 for ROS2 Humble
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rclcpp REQUIRED)
find_package(image_transport REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(OpenCV REQUIRED)

# Generate messages
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Detection2d.msg"
  "msg/Detections2d.msg"
  DEPENDENCIES std_msgs geometry_msgs
)

# Create executable - note the name is different from the project name
add_executable(person_detector_node src/person_detector.cpp)
target_include_directories(person_detector_node PRIVATE
  ${OpenCV_INCLUDE_DIRS}
)
ament_target_dependencies(person_detector_node
  rclcpp
  image_transport
  cv_bridge
  sensor_msgs
  geometry_msgs
  std_msgs
)
target_link_libraries(person_detector_node
  ${OpenCV_LIBRARIES}
)

# Use the new rosidl_get_typesupport_target() approach instead of deprecated rosidl_target_interfaces()
rosidl_get_typesupport_target(cpp_typesupport_target "${PROJECT_NAME}" "rosidl_typesupport_cpp")
target_link_libraries(person_detector_node "${cpp_typesupport_target}")

# Install
install(TARGETS
  person_detector_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
>>>>>>> ccd9c4e (ROS2 Humble migration)
