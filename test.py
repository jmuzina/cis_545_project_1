import threading
import math

# Global variables
num_threads = 64
result_lock = threading.Lock()
global_sum = 0
range_start = 1
range_end = 1073741824

def calculate_fourth_roots(thread_id):
    global global_sum

    local_sum = 0
    for i in range(range_start + thread_id, range_end + 1, num_threads):
        local_sum += math.pow(i, 1/4)

    # Update the global sum while holding the lock
    with result_lock:
        global_sum += local_sum

if __name__ == "__main__":
    # Create and start threads
    threads = []
    for i in range(num_threads):
        thread = threading.Thread(target=calculate_fourth_roots, args=(i,))
        threads.append(thread)
        thread.start()

    # Wait for all threads to finish
    for thread in threads:
        thread.join()

    # Print the result
    print("Sum of fourth roots:", global_sum)