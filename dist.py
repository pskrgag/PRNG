import matplotlib.pyplot as plt
import numpy as np
from scipy.stats import chisquare
from scipy.stats import kstest
import sys
import os

# Test uniform by Kolmagorov
def test(data, file):
    significance_level = 0.05

    count, bins_count = np.histogram(data, bins='auto')
    pdf = count / sum(count)
    cdf = np.cumsum(pdf)
    _, p = kstest(cdf, 'uniform')

    print(f"file {file} p = {p}")
    if p <= significance_level: 
        print(f'Reject null hypothesis {file}') 
    else: 
        print(f'Accept null hypothesis {file}')

data = []

for i in sys.argv[1:]:
    data.append(i)

if len(data) == 0:
    print(f"Usage {sys.argv[0]} <binary files of data>")
    exit(1)

plt.title("Histogram")

for i in data:
    f = open(i, "r")
    data = np.fromfile(f, dtype=np.uint8)
    plt.hist(data, label=os.path.basename(i), histtype='step')
    # test(data, i)

plt.legend(loc='upper right')
plt.show()
