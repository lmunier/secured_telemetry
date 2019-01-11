# Python modules
import time
import mmap
import sys
import hashlib

# 3rd party modules
import posix_ipc

# Utils for this demo
import utils

# Import DroneKit-Python
from dronekit import connect, Command, LocationGlobal
from pymavlink import mavutil
import time, sys, argparse, math


def init_shm_semaphore():
    params = utils.read_params()

    # Create the shared memory and the semaphore.
    memory = posix_ipc.SharedMemory(params["SHARED_MEMORY_NAME"], posix_ipc.O_CREAT,
                                    size=params["SHM_SIZE"])
    semaphore = posix_ipc.Semaphore(params["SEMAPHORE_NAME"], posix_ipc.O_CREAT)

    # MMap the shared memory
    mapfile = mmap.mmap(memory.fd, memory.size)

    # Once I've mmapped the file descriptor, I can close it without
    # interfering with the mmap.
    memory.close_fd()

    # I seed the shared memory with a random string (the current time).
    what_i_wrote = time.asctime()
    utils.write_to_memory(mapfile, what_i_wrote)

    return semaphore, mapfile, what_i_wrote


def destroy_shm_semaphore(semaphore, mapfile):
    params = utils.read_params()

    utils.say("Destroying semaphore and shared memory.")
    mapfile.close()
    # I could call memory.unlink() here but in order to demonstrate
    # unlinking at the module level I'll do it that way.
    posix_ipc.unlink_shared_memory(params["SHARED_MEMORY_NAME"])

    semaphore.release()

    # I could also unlink the semaphore by calling
    # posix_ipc.unlink_semaphore() but I'll do it this way instead.
    semaphore.unlink()


def connection_uav():
    # Connect to the Vehicle
    print "Connecting"
    connection_string = '/dev/ttyAMA0'
    vehicle = connect(connection_string, baud=921600, wait_ready=True)

    return vehicle



def main(semaphore, mapfile, what_i_wrote =""):
    vehicle = connection_uav()
    semaphore.release()
    semaphore.acquire()
    
    s = utils.read_from_memory(mapfile)
    print s

    # I keep checking the shared memory until something new has
    # been written.
    while s == what_i_wrote:
        # Nothing new; give Mrs. Conclusion another chance to respond.
        utils.say("Releasing the semaphore")
        semaphore.release()

        utils.say("Waiting to acquire the semaphore")
        semaphore.acquire()

        s = utils.read_from_memory(mapfile)

    what_i_wrote = vehicle.attitude
    utils.write_to_memory(mapfile, what_i_wrote)

    return what_i_wrote


if __name__ == "__main__":
    params = utils.read_params()
    the_semaphore, the_mapfile, what_i_wrote = init_shm_semaphore()

    for i in range(params["ITERATIONS"]):
        print the_semaphore
        print the_mapfile
        what_i_wrote = main(the_semaphore, the_mapfile, what_i_wrote)

    destroy_shm_semaphore(the_semaphore, the_mapfile)