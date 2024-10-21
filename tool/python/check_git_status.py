import os
import subprocess
import time
import argparse

def get_git_status():
    result = subprocess.run(['git', 'status', '--porcelain'], capture_output=True, text=True)
    if result.returncode != 0:
        raise Exception("Failed to run git status")
    return result.stdout

def generate_timestamp(output_dir, is_output_log = False):
    status = get_git_status()
    is_clean = len(status.strip()) == 0
    status_str = "clean" if is_clean else "dirty"
    
    status_file = os.path.join(output_dir, '.gitstatus')
    timestamp_file = os.path.join(output_dir, '.gitstatus.timestamp')
    
    # Check if the current status is different from the last status
    previous_status = ""
    if os.path.exists(status_file):
        with open(status_file, 'r') as f:
            previous_status = f.read().strip()
    
    if status_str != previous_status or not os.path.exists(timestamp_file):
        # Write the new status to the status file
        with open(status_file, 'w') as f:
            f.write(status_str)
        
        # Update the timestamp file
        with open(timestamp_file, 'w') as f:
            f.write(str(time.time()))
        if is_output_log:
            print(f"Git status changed to '{status_str}'. Timestamp updated.")
    else:
        if is_output_log:
            print("Git status has not changed.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Generate git status information.')
    parser.add_argument('--output-dir', required=True, help='Directory to output the generated information.')
    args = parser.parse_args()
    if not os.path.exists(args.output_dir):
        raise Exception(f"Output directory '{args.output_dir}' does not exist.")
    generate_timestamp(args.output_dir, True)
