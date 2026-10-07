# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os
import re
import shutil
import subprocess
from pathlib import Path

def _read_cmake_version():
    cmake = Path(__file__).parent.parent.parent / 'CMakeLists.txt'
    m = re.search(r'project\s*\([^)]*VERSION\s+([\d.]+)', cmake.read_text())
    return m.group(1) if m else 'unknown'

project = 'DGGRID'
copyright = '2025, Kevin Sahr & DGGRID contributors'
author = 'Kevin Sahr & DGGRID contributors'
release = _read_cmake_version()
version = '.'.join(release.split('.')[:2])

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.todo',
    'myst_parser']

templates_path = ['_templates']

# -- Options for MyST ----------------------------------------------------------
# generate GitHub-style anchors (e.g. #1-introduction) for internal links

myst_heading_anchors = 3
exclude_patterns = [
    'appendix_a.md',  # included into the manual
    '_build',
    'convert',
    'Thumbs.db',
    '.DS_Store'
    ]

# -- Doxygen C++ API reference -----------------------------------------------
# Runs doxygen (documentation/Doxyfile) from the repository root. Its HTML
# output (documentation/build/doxygen/api) is copied into the Sphinx output
# as /api/ via html_extra_path, see cpp.rst. Skipped with a warning if doxygen
# is not installed; set DGGRID_SKIP_DOXYGEN=1 to skip it for quick builds.

_repo_root = Path(__file__).resolve().parents[2]
_doxygen_out = _repo_root / 'documentation' / 'build' / 'doxygen'

if os.environ.get('DGGRID_SKIP_DOXYGEN') != '1':
    if shutil.which('doxygen'):
        shutil.rmtree(_doxygen_out, ignore_errors=True)
        _doxygen_out.mkdir(parents=True)
        subprocess.run(['doxygen', 'documentation/Doxyfile'], cwd=_repo_root,
                       env={**os.environ, 'DGGRID_VERSION': release},
                       check=True)
        # drop doxygen's intermediate graph files, not needed in the output
        for _f in (_doxygen_out / 'api').glob('*.md5'):
            _f.unlink()
        for _f in (_doxygen_out / 'api').glob('*.map'):
            _f.unlink()
    else:
        print('WARNING: doxygen not found, C++ API reference not built')

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# html_theme = 'alabaster'
# readthedocs.org uses sphinx_rtd_theme

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
html_css_files = ['custom.css']
html_extra_path = [str(_doxygen_out)] if (_doxygen_out / 'api').is_dir() else []

# -- Options for LaTeX/PDF output --------------------------------------------
# Appendix A wraps its parameter table in a pdflscape landscape environment.

latex_elements = {
    'preamble': r'''
\usepackage{pdflscape}
\DeclareUnicodeCharacter{2264}{\ensuremath{\leq}}
\DeclareUnicodeCharacter{2265}{\ensuremath{\geq}}
\DeclareUnicodeCharacter{03C6}{\ensuremath{\varphi}}
\DeclareUnicodeCharacter{221A}{\ensuremath{\surd}}
''',
}

