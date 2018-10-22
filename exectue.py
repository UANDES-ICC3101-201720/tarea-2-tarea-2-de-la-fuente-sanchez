import os

algorithms = ["rand", "fifo", "custom"]
programs = ["sort", "scan", "focus"]

for algoritm in algorithms:
	for program in programs:
		for i in range(2,101):
			string = "./virtmem 100 " + str(i) + " " + algoritm + " " + program + " >> " + algoritm + "-" + program + ".txt"
			os.system(string)