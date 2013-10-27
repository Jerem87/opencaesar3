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

#include "cityservice_prosperity.hpp"
#include "city.hpp"
#include "core/safetycast.hpp"
#include "core/position.hpp"
#include "building/house.hpp"
#include "house_level.hpp"
#include "gfx/tile.hpp"
#include "building/entertainment.hpp"
#include "gamedate.hpp"
#include "cityfunds.hpp"
#include "empire.hpp"

class CityServiceProsperity::Impl
{
public:
  CityPtr city;
  DateTime lastDate;
  int prosperity;
  int houseCapTrand;
  int prosperityExtend;
  bool makeProfit;
  int lastYearBalance;
  int worklessPercent;
  int lastYearProsperity;
  int workersSalary;
  int percentPlebs;
};

CityServicePtr CityServiceProsperity::create( CityPtr city )
{
  CityServicePtr ret( new CityServiceProsperity( city ) );
  ret->drop();

  return ret;
}

CityServiceProsperity::CityServiceProsperity( CityPtr city )
  : CityService( getDefaultName() ), _d( new Impl )
{
  _d->city = city;
  _d->lastDate = GameDate::current();
  _d->prosperity = 0;
  _d->houseCapTrand = 0;
  _d->prosperityExtend = 0;
  _d->makeProfit = false;
  _d->lastYearBalance = city->getFunds().getValue();
  _d->worklessPercent = 0;
  _d->workersSalary = city->getFunds().getWorkerSalary();
  _d->lastYearProsperity = 0;
  _d->percentPlebs = 0;
}

void CityServiceProsperity::update( const unsigned int time )
{
  if( time % 44 != 1 )
    return;

  if( abs( GameDate::current().getYear() - _d->lastDate.getYear() ) == 1 )
  {
    _d->lastDate = GameDate::current();

    if( _d->city->getPopulation() == 0 )
    {
      _d->prosperity = 0;
      _d->prosperityExtend = 0;
      return;
    }

    CityHelper helper( _d->city );
    HouseList houses = helper.find<House>( B_HOUSE );

    int prosperityCap = 0;
    int patricianCount = 0;
    int plebsCount = 0;
    foreach( HousePtr house, houses)
    {
      prosperityCap += house->getLevelSpec().getProsperity();
      patricianCount += house->getLevelSpec().isPatrician() ? house->getHabitants().count() : 0;
      plebsCount += house->getLevelSpec().getHouseLevel() < 5 ? house->getHabitants().count() : 0;
    }

    prosperityCap /= houses.size();

    _d->lastYearProsperity = getValue();

    int saveValue = _d->prosperity;
    _d->prosperity = math::clamp( prosperityCap, 0, _d->prosperity + 2 );
    _d->houseCapTrand = _d->prosperity - saveValue;

    int currentFunds = _d->city->getFunds().getValue();
    _d->makeProfit = _d->lastYearBalance < currentFunds;
    _d->lastYearBalance = currentFunds;
    _d->prosperityExtend = (_d->makeProfit ? 2 : -1);

    bool more10PercentIsPatrician = (patricianCount / (float)_d->city->getPopulation()) > 0.1;
    _d->prosperityExtend += (more10PercentIsPatrician ? 1 : 0);

    _d->percentPlebs = plebsCount * 100/ (float)_d->city->getPopulation();
    _d->prosperityExtend += (_d->percentPlebs < 30 ? 1 : 0);

    bool haveHippodrome = !helper.find<Hippodrome>( B_HIPPODROME ).empty();
    _d->prosperityExtend += (haveHippodrome > 0 ? 1 : 0);

    _d->worklessPercent = CityStatistic::getWorklessPercent( _d->city );
    bool unemploymentLess5percent = _d->worklessPercent < 5;
    bool unemploymentMore15percent = _d->worklessPercent > 15;
    _d->prosperityExtend += (unemploymentLess5percent ? 1 : 0);
    _d->prosperityExtend += (unemploymentMore15percent ? -1 : 0);

    bool havePatrician = patricianCount > 0;
    _d->prosperityExtend += (havePatrician ? 1 : 0);

    _d->workersSalary = _d->city->getFunds().getWorkerSalary() - _d->city->getEmpire()->getWorkersSalary();
    _d->prosperityExtend += (_d->workersSalary > 0 ? 1 : 0);
    _d->prosperityExtend += (_d->workersSalary < 0 ? -1 : 0);
   
    bool brokeAndCaesarBailCity = false;
    _d->prosperityExtend += (brokeAndCaesarBailCity ? -3 : 0);

    bool failurePayTribute = false;
    _d->prosperityExtend += (failurePayTribute ? -3 : 0);
  }
}

int CityServiceProsperity::getValue() const
{
  return _d->prosperity + _d->prosperityExtend;
}

int CityServiceProsperity::getMark(CityServiceProsperity::Mark type) const
{
  switch( type )
  {
  case cmHousesCap: return _d->houseCapTrand;
  case cmHaveProfit: return _d->makeProfit;
  case cmWorkless: return _d->worklessPercent;
  case cmWorkersSalary: return _d->workersSalary;
  case cmChange: return getValue() - _d->lastYearProsperity;
  case cmPercentPlebs: return _d->percentPlebs;
  }

  return 0;
}

std::string CityServiceProsperity::getDefaultName()
{
  return "prosperity";
}
