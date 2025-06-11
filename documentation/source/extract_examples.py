import os
import sys
import geopandas as gpd
import numpy as np
import pandas as pd
import re

dggrid_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '../build/src/apps/dggrid/dggrid')

examples_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), '../examples')

filter_folders = ['sampleOutput']

def initial_examples_list():
    examples = []
    for entry in os.listdir(examples_path):
        # if entry is directory, add
        if os.path.isdir(os.path.join(examples_path, entry)) and entry not in filter_folders:
            examples.append(os.path.join(entry))
    return examples


# in the directory there is always one *.meta file
# we need to read it in
# the header of this file is separated by a long line of '#'
# the top part of the header has a textual description
# the bottom part of the header has the meta data
def get_meta_info(entry):
    meta_file = os.path.join(examples_path, entry, entry + '.meta')
    with open(meta_file, 'r') as f:
        meta = f.read()
    
    print(meta)
    # make regex that recognizes a long '#' line
    # count how many of these lines are in the file (usually 2 or 3)
    # solit the file with the last one
    matcher = re.compile(r'#####.*\n')
    matches = matcher.findall(meta)
    print(matches)
    # count how many matches
    c = len(matches)
    print(c)
    # find the index location for the matches
    idx = [m.start() for m in re.finditer(r'#####.*\n', meta)]
    print(idx)
    # take all text before the index for the doc
    # take all text after the index for the params
    doc = meta[:idx[-1]]
    doc = doc.replace('#', '')
    # split into lines '\n' and remove empty lines
    doc = [line.strip() for line in doc.split('\n') if line]
    # join again with '\n'
    doc = '\n'.join(doc)
    # keep the second part as is
    params = meta[idx[-1]:]
    # replace the the last matched string with '# parameters'
    params = params.replace(matches[-1], '# parameters')
    # prepend each row in params with 4 spaces
    params = '\n'.join(['    ' + line for line in params.split('\n') if line])
    return doc, params


# returns the string formatted in restructured text format (rst, NOT markdown) to be concatenated into a sphinx doc
# needs a 2nd level header  with the name of the example, the doc string as normal text,
# and params as code box
def format_meta_info(name, doc, params):
    return f'.. _{name}:\n\n{name}\n{"-"*len(name)}\n\n{doc}\n\n.. code-block:: python\n\n{params}\n\n'
    

def main():
    example_entries = initial_examples_list()
    metas = []
    for entry in example_entries:
        d, m = get_meta_info(entry)
        rst_string = format_meta_info(entry, d, m)
        metas.append(rst_string)

        print(entry)

    # initialize the file with the header
    header = """
.. DGGRID examples folder

DGGRID Examples
===============

"""
    metas.insert(0, header)

    # write to file
    with open('examples.rst', 'w') as f:
        f.write('\n'.join(metas))



if __name__ == '__main__':
    main()

