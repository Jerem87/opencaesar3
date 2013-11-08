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

#include "cityservice_religion.hpp"
#include "city.hpp"
#include "core/safetycast.hpp"
#include "core/position.hpp"
#include "gamedate.hpp"
#include "building/religion.hpp"
#include "divinity.hpp"
#include "events/event.hpp"
#include "core/foreach.hpp"
#include "core/stringhelper.hpp"
#include "core/gettext.hpp"
#include "building/constants.hpp"

using namespace constants;

class CityServiceReligion::Impl
{
public:
  CityPtr city;
  DateTime lastDate;

  void updateRelation( RomeDivinityPtr divn );
};

CityServicePtr CityServiceReligion::create( CityPtr city )
{
  CityServicePtr ret( new CityServiceReligion( city ) );
  ret->drop();

  return ret;
}

CityServiceReligion::CityServiceReligion( CityPtr city )
  : CityService( "religion" ), _d( new Impl )
{
  _d->city = city;
  _d->lastDate = GameDate::current();
}

void CityServiceReligion::update( const unsigned int time )
{
  if( _d->lastDate.getMonth() == GameDate::current().getMonth() )
    return;

  _d->lastDate = GameDate::current();

  DivinePantheon::Divinities divinities = DivinePantheon::getAll();
  foreach( RomeDivinityPtr div, divinities )
  {
    _d->updateRelation( div );
  }
}

void CityServiceReligion::Impl::updateRelation(RomeDivinityPtr divn)
{
  CityHelper helper( city );
  int peopleReached = 0;
  TempleList temples = helper.find<Temple>( building::religionGroup );
  foreach( TemplePtr temple, temples )
  {
    peopleReached += ( temple->getDivinity() == divn ? temple->getParishionerNumber() : 0 );
  }

  float faithIncome = (float)peopleReached / (float)(city->getPopulation()+1);
  divn->updateRelation( faithIncome, city );
}
