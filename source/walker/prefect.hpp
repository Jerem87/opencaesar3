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

#ifndef __OPENCAESAR3_WALKER_PREFECT_H_INCLUDED__
#define __OPENCAESAR3_WALKER_PREFECT_H_INCLUDED__

#include "serviceman.hpp"
#include "building/prefecture.hpp"

class Prefect : public ServiceWalker
{
public:
  static PrefectPtr create( CityPtr city );
 
  virtual void onMidTile();
  virtual void onNewTile();
  virtual void timeStep(const unsigned long time);
  float getServiceValue() const;
  virtual void onDestination();

  virtual void load( const VariantMap& stream );
  virtual void save( VariantMap& stream ) const;

  virtual void send2City( PrefecturePtr prefecture, int water=0 );
  virtual void die();

  ~Prefect();

protected:
  Prefect( CityPtr city );

  bool _looks4Protestor(TilePos& pos);
  bool _looks4Fire( ReachedBuildings& buildings, TilePos& pos );
  void _checkPath2NearestFire( const ReachedBuildings& buildings );
  void _serveBuildings( ReachedBuildings& reachedBuildings );
  void _back2Prefecture();
  void _back2Patrol();

  class Impl;
  ScopedPtr< Impl > _d;
};

#endif //__OPENCAESAR3_WALKER_PREFECT_H_INCLUDED__
