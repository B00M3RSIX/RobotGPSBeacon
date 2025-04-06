Import("env")
import os

# Get the path to the microROS library
project_dir = env.subst("$PROJECT_DIR")
microros_lib_path = os.path.join(project_dir, ".pio", "libdeps", "teensy40", "micro_ros_arduino", "src", "imxrt1062", "fpv5-d16-hard", "libmicroros.a")

# Verify the file exists
if os.path.exists(microros_lib_path):
    print(f"Found microROS library at: {microros_lib_path}")
    
    # Add to libraries directly
    env.Append(LIBS=[File(microros_lib_path)])
else:
    print(f"WARNING: microROS library not found at {microros_lib_path}")