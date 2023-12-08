import matplotlib
import matplotlib.pyplot as plt
import numpy             as np

from IPython.display import display_html

from cycler     import cycler
from contextlib import contextmanager
from itertools  import chain
from itertools  import cycle


def change_display_width(width):
    s  = "from IPython.core.display import display, HTML;"
    s += "display(HTML('<style>.container { width:" + str(width) + "% !important; }</style>'))"
    return s # use exec(s)



def display_side_by_side(*dfs, titles=cycle([''])):
    html_template = '''
<th style="text-align:center"><td style="vertical-align:top">
<h2>{title}</h2>
{df}
</td></th>
'''

    html_str = ""
    for df, title in zip(dfs, chain(titles, cycle(["</br>"]))):
        df_html   = df.to_html().replace("table", 'table style="display:inline"')
        html_str += html_template.format( title = title
                                        , df    = df_html
                                        )

    display_html(html_str, raw=True)



color_sequence = ("k", "m", "g", "b", "r",
                  "gray", "aqua", "gold", "lime", "purple",
                  "brown", "lawngreen", "tomato", "lightgray", "lightpink")

figsize_x = 10
figsize_y =  8

subplots = {
    1 : (1, 1),
    2 : (1, 2),
    3 : (1, 3),
    4 : (2, 2),
    5 : (2, 3),
    6 : (2, 3),
    7 : (3, 3),
    8 : (3, 3),
    9 : (3, 3),
   10 : (5, 2),
}

def auto_plot_style(overrides = dict()):
    plt.rcParams[ "figure.figsize"               ] = 10, 8
    plt.rcParams[   "font.size"                  ] = 25
    plt.rcParams[  "lines.markersize"            ] = 25
    plt.rcParams[  "lines.linewidth"             ] = 3
    plt.rcParams[  "patch.linewidth"             ] = 3
    plt.rcParams[   "axes.linewidth"             ] = 2
    plt.rcParams[   "grid.linewidth"             ] = 3
    plt.rcParams[   "grid.linestyle"             ] = "--"
    plt.rcParams[   "grid.alpha"                 ] = 0.5
    plt.rcParams["savefig.dpi"                   ] = 300
    plt.rcParams["savefig.bbox"                  ] = "tight"
    plt.rcParams[   "axes.formatter.use_mathtext"] = True
    plt.rcParams[   "axes.formatter.limits"      ] = (-3 ,4)
    plt.rcParams[  "xtick.major.size"            ] = 10
    plt.rcParams[  "ytick.major.size"            ] = 10
    plt.rcParams[  "xtick.minor.size"            ] = 5
    plt.rcParams[  "ytick.minor.size"            ] = 5
    plt.rcParams[   "axes.prop_cycle"            ] = cycler(color=color_sequence)
    plt.rcParams[  "image.cmap"                  ] = "gnuplot2"
    plt.rcParams.update(overrides)


@contextmanager
def temporary(name, new_value):
    old_value          = plt.rcParams[name]
    plt.rcParams[name] = new_value
    try    : yield
    finally: plt.rcParams[name] = old_value


def figure(n=1, figsize_x=figsize_x, figsize_y=figsize_y, **kwargs):
    ny, nx = subplots[n]
    return plt.figure(figsize=(nx*figsize_x, ny*figsize_y), **kwargs)


def subplot(a, b=None, c=None):
    if c is None:
        (ny, nx), k = subplots[a], b
    else:
        ny, nx, k = a, b, c
    return plt.subplot(ny, nx, k)


def labels(xlabel=None, ylabel=None, title=None):
    if xlabel is not None: plt.xlabel(xlabel)
    if ylabel is not None: plt.ylabel(ylabel)
    if title  is not None: plt. title( title)


def normhist(x, *args, normto=100, normfactor=None, **kwargs):
    if normfactor is None:
        w = np.full(len(x), normto/len(x))
    else:
        w = np.full(len(x), normfactor)
    return plt.hist(x, *args, weights=w, **kwargs)


def auto_ylimits(data, factor=1.1):
    low   = min(data)
    low  *= factor if low  < 0 else 1/factor
    high  = max(data)
    high *= factor if high > 0 else 1/factor
    plt.ylim(low, high)
