/*******************************************************************************
    Copyright (C) 2023 Kevin Sahr

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
////////////////////////////////////////////////////////////////////////////////
//
// DgHexSF.h: DgHexSF class definitions
//
// Version 6.1 - Kevin Sahr, 5/23/13
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGHEXSF_H
#define DGHEXSF_H

#include <string>
#include <iostream>
#include <dglib/DgIVec3D.h>
#include "SubOpGen.h"

class MainParam;
class GridGenParam;
class DgIDGGSBase;
class DgIDGGBase;
class DgContCartRF;
class DgEvalData;

struct OpBasic;

////////////////////////////////////////////////////////////////////////////////
class DgHexSF {

   public:

      static const std::string all;
      static const std::string cs4;
      static const std::string cs3A;
      static const std::string cs3B;
      static const std::string cs3rA;
      static const std::string cs3rB;
      static const std::string cs3rC;
      static const std::string cs3rD;
      static const std::string cs3rE;
      static const std::string cs3rF;

      DgHexSF (SubOpGen& _genOp)
         : type_ ('A'), res_ (0), ijkCoord_(0, 0, 0), classI_ (true),
           quadNum_ (0), op_ (_genOp.op), genOp_ (_genOp) { }

      DgHexSF (const DgHexSF& h)
         : type_ (h.type_), res_ (h.res_), ijkCoord_(0, 0, 0), classI_ (true),
           quadNum_ (h.quadNum_), op_ (h.op_), genOp_ (h.genOp_) { }

      DgHexSF (SubOpGen& _genOp, const DgIVec3D& ijk, int res = 0, bool classI = true,
               int quadNum = 0)
         : type_ ('A'), res_ (res), ijkCoord_ (ijk), classI_ (classI),
           quadNum_ (quadNum), op_ (_genOp.op), genOp_ (_genOp) { }

      DgHexSF (SubOpGen& _genOp, int i, int j, int k, int res = 0, bool classI = true,
               int quadNum = 0)
         : type_ ('A'), res_ (res), ijkCoord_ (i, j, k), classI_ (classI),
           quadNum_ (quadNum), op_ (_genOp.op), genOp_ (_genOp) { }

     ~DgHexSF (void) { }

      inline operator std::string (void) const;

      inline std::string superFundIndex (void) const;

      const DgIVec3D&    ijkCoord (void) const { return ijkCoord_; }
      char               type     (void) const { return type_; }
      int                res      (void) const { return res_; }
      bool               classI   (void) const { return classI_; }
      const std::string& ciNdx    (void) const { return ciNdx_; }
      const std::string& sfNdx    (void) const { return sfNdx_; }
      int                quadNum  (void) const { return quadNum_; }

      void setIjkCoord (const DgIVec3D& c) { ijkCoord_ = c; }
      void setType     (char c)            { type_ = c; }
      void setRes      (int r)             { res_ = r; }
      void setClassI   (bool classI)       { classI_ = classI; }
      void setCiNdx    (std::string ciNdx) { ciNdx_ = ciNdx; }
      void setSfNdx    (std::string sfNdx) { sfNdx_ = sfNdx; }
      void setQuadNum  (int quadNum)       { quadNum_ = quadNum; }

      OpBasic&  op    (void) { return op_; }
      SubOpGen& genOp (void) { return genOp_; }

      std::string cpiNdx (void) const;

      void addSf3Digit (int digit);

      DgHexSF& operator= (const DgHexSF& h);

      bool operator== (const DgHexSF& h) const
            { return (ijkCoord() == h.ijkCoord() && type() == h.type()
                      && res() == h.res()) && classI() == h.classI()
                      && quadNum() == h.quadNum(); }

      bool operator!= (const DgHexSF& h) const
            { return !operator==(h); }

      unsigned long long int depthFirstTraversal (const DgIDGGSBase& dggs,
                  const DgIDGGBase& dgg, const DgContCartRF& deg, int numAp4Res,
                  DgEvalData* ed = NULL);

      unsigned long long int visitMe (const DgIDGGSBase& dggs,
                  const DgIDGGBase& dgg, const DgContCartRF& deg, DgEvalData* ed);

      DgHexSF downAp4 (void);
      DgHexSF downAp3 (void);
      DgHexSF downAp3r (void);

      DgHexSF dirFromCenter (int digit);

      friend std::ostream& operator<< (std::ostream& stream, const DgHexSF& h);

   private:

      char type_;
      int res_;
      DgIVec3D ijkCoord_;
      bool classI_;
      std::string ciNdx_;
      std::string sfNdx_;
      int quadNum_;

      OpBasic& op_;
      SubOpGen& genOp_;
};

////////////////////////////////////////////////////////////////////////////////
inline std::string
DgHexSF::superFundIndex (void) const
{
   // build the res 1 base tile index out of the quad number and res 1 digit
   int res1tile = 0;
   if (quadNum_ == 0)
   {
      res1tile = 10;
   }
   else
   {
      int res1 = 0;
      if (sfNdx_.length() > 0) res1 = sfNdx_[0] - '1'; // assumes startDigit is 1
      res1tile = 11 + (quadNum_ - 1) * 4 + res1;
   }

   //string completeNdx = dgg::util::to_string(quadNum_, 2) + sfNdx_;

   std::string remainder = "";
   if (sfNdx_.length() > 1) remainder = sfNdx_.substr(1, sfNdx_.length() - 1);

   std::string completeNdx = dgg::util::to_string(res1tile, 2) + remainder;

   return completeNdx;

} // DgHexSF::superFundIndex

////////////////////////////////////////////////////////////////////////////////
inline void
DgHexSF::addSf3Digit (int digit)
// assumes the previous res and this res are both aperture 3
{
   std::string dStr = dgg::util::to_string(digit);
   if (!classI_ || sfNdx_.length() < 1)
      sfNdx_ += dStr;
   else
   {
      std::string pairs[] = {
        "11", "12", "13", "21", "22", "23", "31", "32", "33" };
        //"00", "01", "02", "10", "11", "12", "20", "21", "22" };
      std::string digits[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

      int lastNdx = (int) sfNdx_.length() - 1;
      sfNdx_.append(dStr);
      std::string lastTwo = sfNdx_.substr(lastNdx, 2);
      std::string newDigit = "";
      for (int i = 0; i < 9; i++)
         if (!lastTwo.compare(pairs[i])) newDigit = digits[i];

      sfNdx_ = sfNdx_.substr(0, lastNdx) + newDigit;
   }
}

////////////////////////////////////////////////////////////////////////////////
inline DgHexSF&
DgHexSF::operator= (const DgHexSF& h)
{
   if (this != &h)
   {
      ijkCoord_ = h.ijkCoord();
      type_ = h.type();
      res_ = h.res();
      classI_ = h.classI();
      quadNum_ = h.quadNum();
   }

   return *this;

} // DgHexSF& DgHexSF::operator=

////////////////////////////////////////////////////////////////////////////////
inline DgHexSF::operator std::string (void) const
{
   return std::string("{ ") + type_ + ", " + ((classI_) ? "I" : "II") + ", res " +
                 dgg::util::to_string(res_) + ", quad " +
                 dgg::util::to_string(quadNum_) + std::string(ijkCoord_) + " } " +
                 ciNdx_ + " " + sfNdx_;

} // DgHexSF::operator std::string

////////////////////////////////////////////////////////////////////////////////
inline std::ostream& operator<< (std::ostream& stream, const DgHexSF& h)
            { return stream << std::string(h); }

////////////////////////////////////////////////////////////////////////////////

#endif
