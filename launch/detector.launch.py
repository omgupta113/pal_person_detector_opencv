from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

def generate_launch_description():
    # Declare arguments
    max_rate_arg = DeclareLaunchArgument(
        'max_rate',
        default_value='5.0',
        description='Maximum processing rate'
    )
    
    scale_arg = DeclareLaunchArgument(
        'scale',
        default_value='0.5',
        description='Image scale factor'
    )
    
    image_arg = DeclareLaunchArgument(
        'image',
        default_value='/camera/image',
        description='Input image topic'
    )
    
    transport_arg = DeclareLaunchArgument(
        'transport',
        default_value='raw',
        description='Image transport type'
    )

    # Create node
    person_detector_node = Node(
        package='pal_person_detector_opencv',
        executable='person_detector_node',  # Updated executable name
        name='person_detector',
        output='screen',
        parameters=[{
            'image': LaunchConfiguration('image'),
            'rate': LaunchConfiguration('max_rate'),
            'scale': LaunchConfiguration('scale'),
            'transport': LaunchConfiguration('transport')
        }]
    )

    return LaunchDescription([
        max_rate_arg,
        scale_arg,
        image_arg,
        transport_arg,
        person_detector_node
    ])