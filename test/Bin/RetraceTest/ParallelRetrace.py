import argparse
import subprocess
import sys
import os
import multiprocessing
import glob

def run_in_parallel(program, args, args_batches):
    processes = []
    exit_codes = []

    for batch in args_batches:
        if not batch:
            continue  # Skip empty batches

        # Create a subprocess for each batch
        p = subprocess.Popen([program] + args + batch)
        processes.append(p)

    # Wait for all subprocesses to complete and collect exit codes
    for p in processes:
        p.communicate()
        exit_codes.append(p.returncode)

    return exit_codes

def split_into_batches(args, n):
    """Splits the list of args into n batches."""
    avg = len(args) / float(n)
    batches = []
    last = 0.0

    while last < len(args):
        batches.append(args[int(last):int(last + avg)])
        last += avg

    return batches

def main():
    parser = argparse.ArgumentParser(description="Run OpenEnroth retrace --headless --check-canonical in parallel")
    parser.add_argument("-j", type=int, default=multiprocessing.cpu_count() * 2, help="Number of subprocesses to spawn")
    parser.add_argument("program", help="Path to OpenEnroth binary")
    parser.add_argument("pattern", help="Glob pattern to match traces")

    args = parser.parse_args()

    # Expand the glob pattern to a list of files
    files = glob.glob(args.pattern)

    if not files:
        print(f"No files matched the glob pattern: {args.pattern}")
        sys.exit(1)

    # Split free args into N batches
    batches = split_into_batches(files, args.j)

    # Run the program in N parallel instances
    exit_codes = run_in_parallel(args.program, ["retrace", "--headless", "--check-canonical"], batches)

    # If any subprocess returned a non-zero exit code, return 1
    if any(code != 0 for code in exit_codes):
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main()
