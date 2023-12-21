import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

import torch
import torch.nn as nn
import torch.optim as optim
import torch.nn.functional as F
from torch.utils.data import Dataset
from torch.utils.data import DataLoader

class SiPMDataset_Josh(Dataset):
    def __init__(self, data_path, total_files):
        self.data_path = data_path
        self.total_files = total_files
        self.dataset = []

        for i in range(self.total_files):
            images = np.load(f'{self.data_path}/images_{i}.npy')
            metadata = pd.read_csv(f'{self.data_path}/metadata_{i}.csv')

            for img, meta in zip(images, metadata.values):
                self.dataset.append((img, meta[1:]))

    def __len__(self):
        return len(self.dataset)

    def __getitem__(self, idx):
        image, position = self.dataset[idx]
        image = torch.tensor(image, dtype=torch.float).unsqueeze(0) # Add channel dimension
        position = torch.tensor(position, dtype=torch.float)

        return image, position


class SiPMDataset_Us(Dataset):
    def __init__(self, data_path, max_files=None):
        self.filenames = sorted(glob.glob(os.path.join(data_path, "*.parquet")))
