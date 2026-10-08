# Build the Sphinx docs from this directory:
#   make html
#   make latexpdf

SPHINXOPTS    ?=
# use the repository's .venv if it exists, otherwise sphinx-build from the PATH
VENV_SPHINX   := $(wildcard $(CURDIR)/../.venv/bin/sphinx-build)
SPHINXBUILD   ?= $(if $(VENV_SPHINX),$(VENV_SPHINX),sphinx-build)
SOURCEDIR     = source
BUILDDIR      = source/_build

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

.PHONY: help Makefile

%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
