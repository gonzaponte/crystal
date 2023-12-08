import numpy as np
import matplotlib.pyplot as plt

def normhist(x, *args, factor=100, **kwargs):
    weights = np.full(len(x), 100 / len(x), dtype=float)
    return plt.hist(x, *args, weights=weights, **kwargs)
