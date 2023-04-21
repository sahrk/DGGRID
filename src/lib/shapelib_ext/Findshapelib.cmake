# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

#
# FindSHAPELIB
# ------
#
# Find SHAPELIB (shapelib) include directories and libraries.
#
# This module will set the following variables:
#
#  SHAPELIB_FOUND          - System has SHAPELIB
#  SHAPELIB_INCLUDE_DIRS   - The SHAPELIB include directories
#  SHAPELIB_LIBRARIES      - The libraries needed to use SHAPELIB
#
# This module will also create the "tbb" target that may be used when building
# executables and libraries.

include(FindPackageHandleStandardArgs)

find_path(shapelib_INCLUDE_DIR shapefil.h
  HINTS
    ENV SHAPELIB_ROOT
    ENV SHAPELIB_DIR
    ${SHAPELIB_ROOT_DIR}
  PATH_SUFFIXES
    include
  )

find_library(shapelib_LIBRARY
  NAMES
    libshp
    shp
  HINTS
    ENV SHAPELIB_ROOT
    ENV SHAPELIB_DIR
    ${SHAPELIB_ROOT_DIR}
  PATH_SUFFIXES
    lib
    libs
    Library
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(shapelib
  REQUIRED_VARS shapelib_INCLUDE_DIR shapelib_LIBRARY
  )

if(SHAPELIB_FOUND)
  set(shapelib_INCLUDE_DIRS ${shapelib_INCLUDE_DIR})
  set(shapelib_LIBRARIES ${shapelib_LIBRARY})

  add_library(shapelib SHARED IMPORTED)
  set_target_properties(shapelib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${shapelib_INCLUDE_DIRS}
    IMPORTED_LOCATION ${shapelib_LIBRARIES}
    IMPORTED_IMPLIB ${shapelib_LIBRARIES}
    )

  mark_as_advanced(shapelib_INCLUDE_DIRS shapelib_LIBRARIES)
endif()
