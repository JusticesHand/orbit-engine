# Simply a build python script, aiming at having a single python file
# for the build process as to make it work across all systems. This
# particular file contains the variable names to call the main build
# script (the actual SPIR-V compilation step). Said second file is
# to remain untouched while this one is to be updated to make work
# on the particular machine running the compilation process.
from build_spirv import build

vulkan_bin_path = r'C:\VulkanSDK\1.0.51.0\Bin'

build(vulkan_bin_path)

input('Press enter to continue...')