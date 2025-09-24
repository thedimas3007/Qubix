import os, glob, shutil
from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
pioenv = env["PIOENV"]
project_dir = env["PROJECT_DIR"]
libdeps_dir = env["PROJECT_LIBDEPS_DIR"]
src_file = os.path.join(project_dir, "extra", "glcdfont.c")

def find_glcdfont_candidates():
    patterns = [
        os.path.join(project_dir, "lib", "**", "glcdfont.c"),
        os.path.join(libdeps_dir, pioenv, "*GFX*Library*", "glcdfont.c"),
        os.path.join(libdeps_dir, pioenv, "*Adafruit*GFX*", "glcdfont.c"),
    ]
    found = []
    for pat in patterns:
        found.extend(glob.glob(pat, recursive=True))
    return found

def replace_file(dst):
    print(f"Replacing: {dst}")
    shutil.copyfile(src_file, dst)

if not os.path.exists(src_file):
    print(f"WARN: Source font not found at {src_file}")
else:
    candidates = find_glcdfont_candidates()
    if not candidates:
        print("WARN: glcdfont.c not found in lib/ or .pio/libdeps/** — is Adafruit GFX installed for this env?")
        print(f"Checked under: {os.path.join(libdeps_dir, pioenv)}")
    else:
        replace_file(candidates[0])
