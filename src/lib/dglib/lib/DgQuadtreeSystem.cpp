/*******************************************************************************
    Copyright (C) 2026 Kevin Sahr

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

#include <dglib/DgQuadtreeSystem.h>

void
DgQuadtreeSystem::validateCompatibility (const DgIDGGSBase& dggs)
{
   if (dggs.gridTopo() != dgg::topo::Triangle || !dggs.isPure() ||
       dggs.aperture() != 4)
      ::report("QUADTREE requires TRIANGLE topology with pure aperture 4",
               DgBase::Fatal);
}

DgQuadtreeSystem*
DgQuadtreeSystem::makeSystem (const DgIDGGSBase& dggs, bool extModeInt,
                               const std::string& name)
{
   validateCompatibility(dggs);
   return new DgQuadtreeSystem(dggs, extModeInt, name);
}

DgQuadtreeSystem::DgQuadtreeSystem (const DgIDGGSBase& dggs, bool extModeInt,
                                    const std::string& name)
   : DgHierNdxSystemRFS<DgQuadtreeRF, DgQuadtreeStringRF>(
         dggs, extModeInt, name, 0, 29)
{
}

void
DgQuadtreeSystem::validateResolution (int res)
{
   if (res < 0 || res > 29)
      ::report("QUADTREE supports DGGS resolutions 0 through 29", DgBase::Fatal);
}

int
DgQuadtreeSystem::validateString (const std::string& value, int res)
{
   validateResolution(res);
   if (value.size() != static_cast<std::size_t>(res + 2))
      ::report("QUADTREE digit string requires exactly two root characters "
               "and one digit per DGGS resolution", DgBase::Fatal);
   if (value[0] < '0' || value[0] > '1' || value[1] < '0' || value[1] > '9')
      ::report("QUADTREE root triangle must be 00 through 19", DgBase::Fatal);
   for (std::size_t k = 2; k < value.size(); ++k)
      if (value[k] < '0' || value[k] > '3')
         ::report("QUADTREE child digits must be 0 through 3", DgBase::Fatal);
   return (value[0] - '0') * 10 + value[1] - '0';
}

DgHierNdxIntCoord
DgQuadtreeSystem::toIntCoord (const DgHierNdxStringCoord& c, int gridRes) const
{
   validateResolution(gridRes);
   if (!isNdxRes(gridRes))
      ::report("QUADTREE resolution is outside this system's indexed range", DgBase::Fatal);
   const std::string& value = c.value();
   const int root = validateString(value, gridRes);
   uint64_t bits = static_cast<uint64_t>(root) << 59;
   for (int r = 1; r <= gridRes; ++r)
      bits |= static_cast<uint64_t>(value[r + 1] - '0') << (59 - 2 * r);
   return DgHierNdxIntCoord(bits);
}

DgHierNdxStringCoord
DgQuadtreeSystem::toStringCoord (const DgHierNdxIntCoord& c, int gridRes) const
{
   validateResolution(gridRes);
   if (!isNdxRes(gridRes))
      ::report("QUADTREE resolution is outside this system's indexed range", DgBase::Fatal);
   const uint64_t bits = c.value();
   const int root = static_cast<int>(bits >> 59);
   if (root > 19)
      ::report("QUADTREE root triangle must be 00 through 19", DgBase::Fatal);
   // At resolution 0 all 59 path bits are padding; at 29 only bit 0 is.
   const uint64_t paddingMask = (uint64_t(1) << (59 - 2 * gridRes)) - 1;
   if (bits & paddingMask)
      ::report("QUADTREE unused bits must be zero", DgBase::Fatal);
   std::string value = dgg::util::to_string(root, 2);
   for (int r = 1; r <= gridRes; ++r)
      value += static_cast<char>('0' + ((bits >> (59 - 2 * r)) & 3));
   return DgHierNdxStringCoord(value);
}

void
DgQuadtreeSystem::setAddNdxParent (const DgResAdd<DgHierNdx>& add,
                                   DgLocation& parent) const
{
   const std::string& value = add.address().strNdx().value();
   validateString(value, add.res());
   DgResAdd<DgHierNdx> pAdd;
   initNdxFromString(pAdd, add.res() - 1, value.substr(0, value.size() - 1));
   forceAddress(&parent, pAdd);
}

void
DgQuadtreeSystem::setAddNdxChildren (const DgResAdd<DgHierNdx>& add,
                                     DgLocVector& children) const
{
   const std::string& value = add.address().strNdx().value();
   validateString(value, add.res());
   children.clearAddress();
   for (char digit = '0'; digit <= '3'; ++digit) {
      DgResAdd<DgHierNdx> chAdd;
      initNdxFromString(chAdd, add.res() + 1, value + digit);
      children.addressVec().push_back(new DgAddress<DgResAdd<DgHierNdx>>(chAdd));
   }
}
