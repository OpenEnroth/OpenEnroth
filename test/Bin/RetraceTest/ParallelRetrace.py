import argparse
import subprocess
import sys
import os
import multiprocessing
import glob
import threading
from queue import Queue

def worker(program, args, queue, results):
    while True:
        batch = queue.get()
        if batch is None:
            break

        # Run the program with the current batch
        p = subprocess.Popen([program] + args + batch, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stdout, stderr = p.communicate()

        # Output the stdout and stderr immediately
        if stdout:
            sys.stdout.write(stdout.decode())
        if stderr:
            sys.stderr.write(stderr.decode())

        # Store the result (exit code)
        results.append(p.returncode)

        queue.task_done()

def split_into_chunks(files, chunk_size):
    """Splits the list of files into chunks of the given size."""
    return [files[i:i + chunk_size] for i in range(0, len(files), chunk_size)]

def main():
    parser = argparse.ArgumentParser(description="Run OpenEnroth retrace --headless --check-canonical in parallel")
    parser.add_argument("-j", type=int, default=multiprocessing.cpu_count(), help="Number of subprocesses to spawn")
    parser.add_argument("--job-size", type=int, default=5, help="Number of traces per job")
    parser.add_argument("--ls", help="Directory to look for traces to retrace")
    parser.add_argument("--headless", action="store_true", help="Run in headless mode")
    parser.add_argument("program", help="Path to OpenEnroth binary")
    parser.add_argument("traces", nargs=argparse.REMAINDER, help="Trace files to retrace")

    args = parser.parse_args()

    # Expand the glob pattern to a list of files
    traces = args.traces
    if args.ls:
        traces += glob.glob(args.ls + "/*.json")
    traces.sort() # We want determinism

    if not traces:
        print("No traces to retrace")
        sys.exit(1)

    # Split the files into chunks
    chunks = split_into_chunks(traces, args.job_size)

    # Create a queue and results list
    queue = Queue()
    results = []

    # Prepare args
    workerArgs = ["retrace", "--check-canonical"]
    if args.headless:
        workerArgs.append("--headless")

    # Start worker threads
    threads = []
    for _ in range(args.j):
        t = threading.Thread(target=worker, args=(args.program, workerArgs, queue, results))
        t.start()
        threads.append(t)

    # Add chunks to the queue
    for chunk in chunks:
        queue.put(chunk)

    # Block until all tasks are done
    queue.join()

    # Stop the worker threads
    for _ in range(args.j):
        queue.put(None)
    for t in threads:
        t.join()

    # If any subprocess returned a non-zero exit code, return 1
    if any(code != 0 for code in results):
        sys.exit(1)
    else:
        sys.exit(0)

if __name__ == "__main__":
    main()
