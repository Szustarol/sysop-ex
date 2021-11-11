#!/usr/bin/python3

import subprocess
import matplotlib.pyplot as plt

program = "./invert.run {} {} body3.ascii.pgm out.ascii.pgm"

n_threads = list(range(1, 25))
numbers_times = []
blocks_times = []


N_CHECKS = 200

for nThreads in n_threads:
	#execute N_CHECKS times and average
	avgNumbers = 0
	for i in range(N_CHECKS):
		out = subprocess.check_output(program.format(nThreads, "numbers").split(' ')).decode("utf-8")
		timePos = out.find(":") 
		time = int(out[timePos+1:])
		avgNumbers += time
	avgNumbers /= N_CHECKS
	avgBlocks = 0
	for i in range(N_CHECKS):
		out = subprocess.check_output(program.format(nThreads, "blocks").split(' ')).decode("utf-8")
		timePos = out.find(":") 
		time = int(out[timePos+1:])
		avgBlocks += time
	avgBlocks /= N_CHECKS

	numbers_times.append(avgNumbers)
	blocks_times.append(avgBlocks)


plt.plot(n_threads, blocks_times, label="Blocks mode")
plt.plot(n_threads, numbers_times, label="Numbers mode")
plt.xlabel("Number of threads")
plt.ylabel("Average execution time over {} executions".format(N_CHECKS))
plt.legend()
plt.show()