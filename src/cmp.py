import subprocess
import sys

cmd = sys.argv[2] + " " + sys.argv[3]
a = subprocess.check_output(cmd, shell=True).strip()
with open(sys.argv[1], 'rb') as fp:
	b = fp.read().strip()

total = min(len(a), len(b))
same = 0
for i in range(total):
	if a[i] == b[i]:
		same = same+1;
	#else:
		#print(chr(a[i])+'->'+chr(b[i]))
print(str(same)+"/"+str(total-1)+":"+str(same/(total-1)*100)+"%")
