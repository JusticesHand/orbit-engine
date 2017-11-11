# Build python script meant to be called from the build.py file bundled
# with the current file. Essentially builds the shaders in this
# directory (hard-coded, as it's a simple utility script) in a mostly
# platform-independant manner.
from os import path
from subprocess import run, PIPE

def build(bin_path):
    validator = path.join(bin_path, 'glslangValidator')
    input_names = ['animated.vert', 'instance.vert', 'default.frag']
    output_names = ['animatedVert.spv', 'instanceVert.spv', 'defaultFrag.spv']
    for input, output in zip(input_names, output_names):
        res = run([validator, '-V', '-o', output, input], stdout = PIPE)
        print(input, '=>', output)
        print(res.stdout.decode('utf-8'))