import re
import glob
base_dir=r"C:\Steam\steamapps\workshop\content\294100\2564129100"
filenames = glob.glob(base_dir + '\\Defs\\**\\*.xml', recursive=True)
print(filenames)