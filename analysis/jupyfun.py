import os
import glob
import itertools

import numpy  as np

from typing import Any
from typing import Union
from typing import Mapping
from typing import Sequence


def progressbar(iterable, *, flushmod=1, nelements=None, index=False):
    if nelements is None:
        temp, iterable = itertools.tee(iterable)
        nelements = sum(1 for _ in temp)
        
    for i, value in enumerate(iterable):
        if i % flushmod == 0:
            print(f"\rItem {i+1} of {nelements}", end="", flush=True)
        if index:
            yield i, value
        else:
            yield value
    print()

def filter_df(df, sel=None, **kwargs):
    if sel is None:
        sel = dict()
    
    sel.update(kwargs)
    
    for column, value in sel.items():
        df = df.loc[getattr(df, column) == value]
    return df


def floatarray(string):
    return np.array(list(map(float, string.split())))


def cast(value):
    try:
        return int(value)
    except:
        try:
            return float(value)
        except:
            return value

        
def meta_from_filename(filename : str, skip : Union[int, Sequence[int]] = None) -> Mapping[str, Any]:
    basename = os.path.basename(filename)
    tokens   = basename.split("_")

    if skip is not None:
        if isinstance(skip, int):
            skip = [skip]
        for i in sorted(skip, reverse=True):
            del tokens[i]

    keys   = tokens[0::2]
    values = tokens[1::2]
    return dict(zip(keys, map(cast, values)))


def from_webplotdigitizer(string):
    """
        Take the output from webplotdigitizer and return the two arrays.
    """
    tokens = string.replace(",", ".").replace(";", " ").replace("\n", " ").split()
    tokens = np.fromiter(map(float, tokens), dtype=float)
    tokens_0 = tokens[ ::2]
    tokens_1 = tokens[1::2]
    return tokens_0, tokens_1


def sorted_filenames(*args, **kwargs):
    return sorted(glob.glob(os.path.join(*args)), **kwargs)


def in_range(x, xmin, xmax, left_closed=True, right_closed=False):
    left  = x >= xmin if  left_closed else x > xmin
    right = x <= xmax if right_closed else x < xmax
    return left & right

