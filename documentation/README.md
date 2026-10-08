# Building the DGGRID documentation

The user manual and C++ API pages are a Sphinx project in `documentation/source`.

## Read the Docs

[Read the Docs](https://readthedocs.org) builds the **HTML** site itself on each
push, using `.readthedocs.yaml` at the repository root. You do not need to run
`make html` locally, and you should not commit `_build/`.

Local HTML is only for preview. Read the Docs does not build the PDF; produce
that locally if you want a printable manual.

## Local preview (HTML)

From the repository root:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r documentation/requirements.txt
cd documentation
make html
```

Open `documentation/source/_build/html/index.html`. Run `make` from
`documentation/` or `documentation/source/` (not the repository root).

A full HTML build also runs Doxygen (needs `doxygen` and `graphviz`) and copies
the API into `_build/html/api/`. To skip that:

```bash
DGGRID_SKIP_DOXYGEN=1 make html
```

## Local PDF

Needs a TeX Live install with `pdflatex` and `latexmk`, plus the Python
environment above.

```bash
cd documentation
make latexpdf
```

The PDF is `documentation/source/_build/latex/dggrid.pdf`. Appendix A is
typeset in landscape.

To skip Doxygen:

```bash
DGGRID_SKIP_DOXYGEN=1 make latexpdf
```
