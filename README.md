# PAL Person Detector OpenCV

A ROS2 Humble package for real-time person detection using OpenCV's HOG (Histogram of Oriented Gradients) detector.

## Overview

This package provides a lightweight solution for detecting people in camera feeds using computer vision. It's based on OpenCV's HOG detector and has been optimized for real-time performance on CPU.

The detector:
- Processes incoming image streams from ROS2 topics
- Detects people in the images using HOG features
- Publishes detection results as bounding boxes
- Provides visualized results for debugging

## Installation

### Prerequisites

- ROS2 Humble
- OpenCV
- Image transport plugins

```bash
# Install dependencies
sudo apt update
sudo apt install -y ros-humble-image-transport ros-humble-cv-bridge ros-humble-image-transport-plugins
```

### Building

```bash
# Clone into your ROS2 workspace
cd ~/ros2_ws/src
git clone https://github.com/username/pal_person_detector_opencv.git

# Build
cd ~/ros2_ws
colcon build --packages-select pal_person_detector_opencv

# Source the workspace
source install/setup.bash
```

## Usage

### Basic Usage

1. Start a camera node
```bash
ros2 run v4l2_camera v4l2_camera_node
```

2. Run the person detector
```bash
ros2 launch pal_person_detector_opencv detector.launch.py
```

3. View results
```bash
ros2 run rqt_image_view rqt_image_view
# Select the "/debug" topic from the dropdown
```

### Parameters

The detector supports several parameters to adjust performance:

- **image**: Input image topic (default: `/image_raw`)
- **scale**: Image scaling factor (default: `0.5`, smaller = faster but less accurate)
- **rate**: Maximum processing rate in Hz (default: `5.0`)
- **transport**: Image transport type (default: `raw`)

Example:
```bash
ros2 launch pal_person_detector_opencv detector.launch.py image:=/camera/image scale:=0.4 rate:=3
```

## Outputs

The detector publishes to the following topics:

- **/detections**: Detection results as an array of bounding boxes
- **/debug**: Visualization of detections drawn on the input image

## Applications

This person detector can be used in various scenarios:

- Robot navigation around humans
- Human-robot interaction systems
- Surveillance and security
- People counting and tracking
- Activity monitoring

## License

This package is licensed under the BSD License. See the LICENSE file for details.

## Credits

Originally developed by PAL Robotics, migrated to ROS2 Humble.