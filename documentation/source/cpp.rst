C++ API reference
=================

The C++ source code documentation is generated with Doxygen from the
libraries in ``src/lib``: ``dglib``, the discrete global grid library, and
``dgaplib``, the operation and parameter-list classes used by the **DGGRID**
application. It includes class, collaboration, and include diagrams. Bundled
third-party code (shapelib, the PROJ.4 gnomonic projection code) is not
included.

`Open the C++ API reference <api/index.html>`_

The reference is built with every documentation build. To build it locally,
run ``doxygen documentation/Doxyfile`` from the repository root (requires
Doxygen and Graphviz); the output is written to
``documentation/build/doxygen/api/index.html``. A Sphinx build of the
documentation runs Doxygen automatically if it is installed; set the
environment variable ``DGGRID_SKIP_DOXYGEN=1`` to skip it.
