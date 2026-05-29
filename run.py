import os
import subprocess
import sys
import shutil

def main():
    build_dir = "build"
    disk_img = "disk.img"
    
    # Auto-create a 1MB raw virtual disk if it doesn't exist
    if not os.path.exists(disk_img):
        print(f"Creating virtual disk: {disk_img}...")
        with open(disk_img, "wb") as f:
            f.seek((1024 * 1024) - 1)
            f.write(b'\0')
            
    if os.path.exists(build_dir):
        shutil.rmtree(build_dir)
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
        "qemu-system-i386",
        "-kernel", f"{build_dir}/bmks.bin",
        "-m", "512M",
        "-no-reboot",
        "-hda", disk_img # Fixed: splitted argument and value
    ]
    subprocess.run(qemu_cmd)

if __name__ == "__main__":
    main()