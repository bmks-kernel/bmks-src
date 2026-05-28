import os
import subprocess
import sys

def main():
    build_dir = "build"
    if not os.path.exists(build_dir):
        os.mkdir(build_dir)
    
    cmake_res = subprocess.run(["cmake", "-S", ".", "-B", build_dir])
    if cmake_res.returncode != 0:
        print("CMake config failed")
        sys.exit(1)
        
    build_res = subprocess.run(["cmake", "--build", build_dir])
    if build_res.returncode != 0:
        print("Build failed")
        sys.exit(1)
        
    qemu_cmd = [
        "qemu-system-x86_64",
        "-kernel", f"{build_dir}/bmks.bin",
        "-m", "512M",
        "-no-reboot"
    ]
    subprocess.run(qemu_cmd)

if __name__ == "__main__":
    main()