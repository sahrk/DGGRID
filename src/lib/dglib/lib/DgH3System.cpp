/*******************************************************************************
    Copyright (C) 2026 DGGRID contributors

    This file is part of DGGRID.

    DGGRID is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DGGRID is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/

// USE_H3 is set in MakeIncludes / CMake
#ifdef USE_H3

#include <dglib/DgH3System.h>

namespace {
void unimplemented (const char* operation)
{
   ::report(std::string("H3 ") + operation +
            " is not implemented (mixed-aperture index math stub)", DgBase::Fatal);
}
}

void
DgH3System::validateCompatibility (const DgIDGGSBase& dggs,
                                    DgHierNdxFormType form)
{
   if (form != Int64 && form != DigitString)
      ::report("H3 requires INT64 or DIGIT_STRING form", DgBase::Fatal);
   if (dggs.gridTopo() != Hexagon)
      ::report("H3 requires HEXAGON topology", DgBase::Fatal);
   if (dggs.isPure())
      ::report("H3 requires a mixed aperture DGGS", DgBase::Fatal);
   if (dggs.nRes() < 1 || dggs.nRes() > 18)
      ::report("H3 supports resolutions 0 through 17", DgBase::Fatal);

   // The configured grid may stop at any resolution of the fixed sequence.
   for (int res = 1; res < dggs.nRes(); ++res) {
      const unsigned int expected = res == 1 ? 4 : (res == 2 ? 3 : 7);
      if (dggs.idggBase(res).aperture() != expected)
         ::report("H3 requires aperture sequence 43 followed by fifteen 7s; "
                  "mismatch at resolution " + std::to_string(res), DgBase::Fatal);
   }
}

DgH3System*
DgH3System::makeSystem (const DgIDGGSBase& dggs, bool extModeInt,
                        const std::string& name)
{
   validateCompatibility(dggs, extModeInt ? Int64 : DigitString);
   return new DgH3System(dggs, extModeInt, name);
}

DgH3System::DgH3System (const DgIDGGSBase& dggs, bool extModeInt,
                        const std::string& name)
   : DgHierNdxSystemRFS<DgH3IntRF, DgH3StringRF>(dggs, extModeInt, name)
{ }

DgHierNdxIntCoord
DgH3IntRF::quantify (const DgQ2DICoord&) const
{
   unimplemented("Q2DI-to-INT64 conversion");
   return undefAddress();
}

DgQ2DICoord
DgH3IntRF::invQuantify (const DgHierNdxIntCoord&) const
{
   unimplemented("INT64-to-Q2DI conversion");
   return DgQ2DICoord::undefDgQ2DICoord;
}

const char*
DgH3IntRF::str2add (DgHierNdxIntCoord*, const char*, char) const
{
   unimplemented("INT64 input parsing and validation");
   return nullptr;
}

DgHierNdxStringCoord
DgH3StringRF::quantify (const DgQ2DICoord&) const
{
   unimplemented("Q2DI-to-DIGIT_STRING conversion");
   return undefAddress();
}

DgQ2DICoord
DgH3StringRF::invQuantify (const DgHierNdxStringCoord&) const
{
   unimplemented("DIGIT_STRING-to-Q2DI conversion");
   return DgQ2DICoord::undefDgQ2DICoord;
}

const char*
DgH3StringRF::str2add (DgHierNdxStringCoord*, const char*, char) const
{
   unimplemented("DIGIT_STRING input parsing and validation");
   return nullptr;
}

DgHierNdxIntCoord
DgH3System::toIntCoord (const DgHierNdxStringCoord&, int) const
{
   unimplemented("DIGIT_STRING-to-INT64 conversion");
   return DgHierNdxIntCoord();
}

DgHierNdxStringCoord
DgH3System::toStringCoord (const DgHierNdxIntCoord&, int) const
{
   unimplemented("INT64-to-DIGIT_STRING conversion");
   return DgHierNdxStringCoord();
}

void
DgH3System::setAddNdxParent (const DgResAdd<DgHierNdx>&, DgLocation&) const
{
   unimplemented("indexing parent");
}

void
DgH3System::setAddNdxChildren (const DgResAdd<DgHierNdx>&, DgLocVector&) const
{
   unimplemented("indexing children");
}

#endif // USE_H3
