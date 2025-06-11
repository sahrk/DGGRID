# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import sys

project = 'DGGRID'
copyright = '2025, Kevin Sahr & DGGRID contributors'
author = 'Kevin Sahr & DGGRID contributors'
release = '8.4.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

# should be in path
# sys.path.append("/home/me/docproj/ext/breathe/")
# sphinx.ext.imgmath s the successor of sphinx.ext.pngmath
extensions = [
    'sphinx.ext.imgmath',
    'sphinx.ext.todo',
    # 'breathe',
    'myst_parser']

templates_path = ['_templates']
exclude_patterns = [
    '_build',
    'convert',
    'Thumbs.db',
    '.DS_Store'
    ]

# -- Options for breathe -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-breathe
# expand conf.py directory and add subdirectory src/docs/xml

# breathe_projects = {"DGLIB": "../build/docs/xml"}
# breathe_default_project = "DGLIB"

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# html_theme = 'alabaster'
# readthedocs.org uses sphinx_rtd_theme

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

