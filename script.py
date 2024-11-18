import os
import shutil
import subprocess
from subprocess import DEVNULL

# List of benchmark names
benchmarks = [
    'perlbench_s', 'gcc_s', 'bwaves_s', 'mcf_s', 'cactuBSSN_s',
    'deepsjeng_s', 'lbm_s', 'omnetpp_s', 'wrf_s', 'xalancbmk_s',
    'specrand_is', 'specrand_fs', 'cam4_s', 'pop2_s', 'imagick_s',
    'nab_s', 'fotonik3d_s', 'roms_s', 'x264_s', 'leela_s',
    'exchange2_s', 'xz_s', 'perlbench', 'bzip2', 'gcc',
    'bwaves', 'gamess', 'mcf', 'milc', 'zeusmp',
    'gromacs', 'cactusADM', 'leslie3d', 'namd', 'gobmk',
    'dealII', 'soplex', 'povray', 'calculix', 'hmmer',
    'sjeng', 'GemsFDTD', 'libquantum', 'h264ref', 'tonto',
    'lbm', 'omnetpp', 'astar', 'wrf', 'sphinx3',
    'xalancbmk', 'specrand_i', 'specrand_f'
]
total = len(benchmarks)

# Directory where the stats.txt file is located
output_dir = "m5out"
stats_file = os.path.join(output_dir, "stats.txt")

# Directory to save the renamed stats files
stats_dir = "stats"
os.makedirs(stats_dir, exist_ok=True)

# Path to the gem5 executable and script
gem5_executable = "./build/ECE565-ARM/gem5.opt"
gem5_script = "configs/spec/spec_se.py"

# Specific CPU to simulate
cpu_type = "MinorCPU"

# Number of instructions to simulate
max_insts = 10000000

# ANSI color codes
BLUE = "\033[94m"
YELLOW = "\033[93m"
GREEN = "\033[92m"
RED = "\033[91m"
RESET = "\033[0m"

# Iterate through the list of benchmarks
for index, benchmark in enumerate(benchmarks):
    print(f"{YELLOW}Running benchmark ({index}/{total}): {benchmark}{RESET}")
    # Construct the command
    cmd = [
        gem5_executable,
        gem5_script,
        "-b", benchmark,
        f"--cpu-type={cpu_type}",
        f"--maxinsts={max_insts}",
        "--l1d_size=64kB",
        "--l1i_size=16kB",
        "--caches",
        "--l2cache"
    ]

    # Run the command and suppress output
    try:
        subprocess.run(cmd, stdout=DEVNULL, stderr=DEVNULL, check=True)
        # Check if stats.txt exists
        if os.path.exists(stats_file):
            # Rename and move stats.txt
            new_file = os.path.join(stats_dir, f"{benchmark}.stats")
            shutil.move(stats_file, new_file)
            print(f"{GREEN}✓ Saved stats for {benchmark} as {new_file}{RESET}")
        else:
            print(f"{RED}✗ Stats file not found for {benchmark}.{RESET}")
    except subprocess.CalledProcessError:
        print(f"{RED}✗ Error running {benchmark}. Skipping...{RESET}")
