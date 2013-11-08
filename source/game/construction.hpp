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
//
// Copyright 2012-2013 Gregoire Athanase, gathanase@gmail.com

#ifndef _OPENCAESAR_CONSTRUCTION_H_INCLUDE_
#define _OPENCAESAR_CONSTRUCTION_H_INCLUDE_

#include "gfx/tileoverlay.hpp"
#include "core/scopedptr.hpp"
#include "core/referencecounted.hpp"
#include "core/predefinitions.hpp"
#include "game/service.hpp"
#include "building/metadata.hpp"

class Construction : public TileOverlay
{
public:
  typedef enum { fire=0, damage } Param;
  Construction( const TileOverlay::Type type, const Size& size );
  virtual ~Construction();

  virtual bool canBuild( CityPtr city, const TilePos& pos ) const;  // returns true if it can be built there
  virtual std::string getError() const;
  virtual void build( CityPtr city, const TilePos& pos );
  virtual void burn();
  virtual void collapse();
  virtual bool isNeedRoadAccess() const;
  virtual const TilemapTiles& getAccessRoads() const;  // return all road tiles adjacent to the construction
  virtual void computeAccessRoads();  
  virtual int  getRoadAccessDistance() const; // virtual because HOUSE has different behavior
  virtual const MetaData::Desirability& getDesirabilityInfo() const;
  virtual void destroy();
  virtual void updateState( Param param, double value, bool relative=true );
  virtual double getState( Param param ) const;
  virtual TilePos getEnterPos() const;
  virtual void timeStep(const unsigned long time);

  virtual void save(VariantMap& stream) const;
  virtual void load(const VariantMap& stream);
protected:
  class Impl;
  ScopedPtr< Impl > _d;
};

#endif//_OPENCAESAR_CONSTRUCTION_H_INCLUDE_
