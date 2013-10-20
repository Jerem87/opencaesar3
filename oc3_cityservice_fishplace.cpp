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

#include "oc3_cityservice_fishplace.hpp"
#include "oc3_city.hpp"
#include "oc3_safetycast.hpp"
#include "oc3_tilemap.hpp"
#include "oc3_positioni.hpp"
#include "oc3_tile.hpp"
#include "oc3_tileoverlay_factory.hpp"
#include "oc3_fish_place.hpp"

class CityServiceFishPlace::Impl
{
public:
  CityPtr city;
  unsigned int maxFishPlace;

  std::vector< FishPlacePtr > places;
};

CityServicePtr CityServiceFishPlace::create( CityPtr city )
{
  CityServicePtr ret( new CityServiceFishPlace( city ) );
  ret->drop();

  return ret;
}

CityServiceFishPlace::CityServiceFishPlace( CityPtr city )
: CityService( "fishplace" ), _d( new Impl )
{
  _d->city = city;
  _d->maxFishPlace = 1;
}

void CityServiceFishPlace::update( const unsigned int time )
{  
  if( time % 44 != 1 )
    return;

  while( _d->places.size() < _d->maxFishPlace )
  {
    TileOverlayPtr fishplace = TileOverlayFactory::getInstance().create( wtrFishPlace );

    if( fishplace != 0 )
    {
      fishplace->build( _d->city, _d->city->getBorderInfo().boatEntry );
      _d->city->addOverlay( fishplace );
      _d->places.push_back( fishplace.as<FishPlace>() );
    }
  }

  std::vector< FishPlacePtr >::iterator fit = _d->places.begin();
  while( fit != _d->places.end() )
  {
    if( (*fit)->isDeleted() )
    {
      fit = _d->places.erase( fit );
    }
    else
    {
      fit++;
    }
  }
}
