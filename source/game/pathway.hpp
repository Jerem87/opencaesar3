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

#ifndef __OPENCAESAR3_PATHWAY_H_INCLUDED__
#define __OPENCAESAR3_PATHWAY_H_INCLUDED__

#include "core/predefinitions.hpp"
#include "core/position.hpp"
#include "core/variant.hpp"
#include "core/scopedptr.hpp"
#include "core/direction.hpp"

class Tilemap;

class PathWay
{
public:
  PathWay();
  PathWay( const PathWay& copy );

  virtual ~PathWay();

  void init( const Tilemap &tilemap, const Tile &origin );

  int getLength() const;
  const Tile& getOrigin() const;
  const Tile& getDestination() const;
  bool isReverse() const;
  unsigned int getStep() const;

  void begin();
  void rbegin();
  virtual constants::Direction getNextDirection();
  bool isDestination() const;

  void setNextDirection(constants::Direction direction);
  void setNextTile( const Tile& tile);
  bool contains(Tile &tile);
  ConstTilemapTiles& getAllTiles();

  void prettyPrint() const;
  void toggleDirection();

  PathWay& operator=(const PathWay& other );

  void load( const VariantMap& stream );
  VariantMap save() const;

  bool isValid() const;
private:
  Tilemap const* _tilemap;
  Tile const* _origin;

  class Impl;
  ScopedPtr< Impl > _d;
};

bool operator<(const PathWay &v1, const PathWay &v2);

#endif //__OPENCAESAR3_PATHWAY_H_INCLUDED__
