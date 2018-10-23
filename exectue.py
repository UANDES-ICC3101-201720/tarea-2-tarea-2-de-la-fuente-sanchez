import os
import numpy as np
import matplotlib.pyplot as plt

def get_data(file_data):
	file_data = open(file_data)
	data = []
	while(True):
		trash = file_data.readline()
		results = file_data.readline().strip()
		if results == '':
			break
		results = [int(i) for i in results.split(",")]
		data.append(results)
	file_data.close()
	return data
"""
algorithms = ["rand", "fifo", "custom"]
programs = ["sort", "scan", "focus"]

for algoritm in algorithms:
	for program in programs:
		for i in range(2,101):
			string = "./virtmem 100 " + str(i) + " " + algoritm + " " + program + " >> " + algoritm + "-" + program + ".txt"
			os.system(string)
"""
data_files = { "rand-scan.txt": "",
			   "fifo-scan.txt": "",
			   "custom-scan.txt": "",
			   "rand-sort.txt": "",
               "fifo-sort.txt": "",
               "custom-sort.txt": "",
               "rand-focus.txt": "",
               "fifo-focus.txt": "",
               "custom-focus.txt": "" }

for file_ in data_files:
	data_files[file_] = get_data(file_)

# Make some fake data.
a = b = np.arange(0, 3, .02) 
c = np.exp(a)
d = c[::-1]

nframes = []
n_writes = []
n_reads = []
n_page_faults = []
for data in data_files["custom-sort.txt"]:
	nframes.append(data[0])
	n_writes.append(data[1])
	n_reads.append(data[2])
	n_page_faults.append(data[3])
	print(data[0], data[1], data[2],data[3])
# Create plots with pre-defined labels.
fig, ax = plt.subplots()
ax.plot(nframes, n_page_faults, 'k--', label='Page Faults')
ax.plot(nframes, n_writes, 'k:', label='Writes')
ax.plot(nframes, n_reads, 'k', label='Reads')

legend = ax.legend(loc='upper center', shadow=True, fontsize='x-large')

# Put a nicer background color on the legend.
legend.get_frame().set_facecolor('C0')

plt.show()