// This file is part of openCaesar3.
//
// openCaesar3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openCaesar3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openCaesar3.  If not, see <http://www.gnu.org/licenses/>.

#ifndef _OPENCAESAR3_WHARF_INCLUDE_H_
#define _OPENCAESAR3_WHARF_INCLUDE_H_

#include "oc3_building_factory.hpp"

class Wharf : public Factory
{
public:
  Wharf();
  virtual bool canBuild(CityPtr city, const TilePos& pos ) const;  // returns true if it can be built there
  virtual void build(CityPtr city, const TilePos &pos);
  virtual void destroy();

private:
  void _setPicture( DirectionType direction );

private:
  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //_OPENCAESAR3_WHARF_INCLUDE_H_
