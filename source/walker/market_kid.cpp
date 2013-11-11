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

#include "market_kid.hpp"
#include "market_lady.hpp"
#include "building/market.hpp"
#include "game/city.hpp"
#include "game/pathway.hpp"
#include "game/goodstore.hpp"
#include "game/resourcegroup.hpp"
#include "game/name_generator.hpp"
#include "constants.hpp"
#include "corpse.hpp"

using namespace constants;

class MarketKid::Impl
{
public:
  GoodStock basket;
  unsigned long delay;
  TilePos marketPos;
  unsigned long birthTime;
};

MarketKidPtr MarketKid::create( CityPtr city, MarketLadyPtr lady )
{
  MarketKidPtr ret( new MarketKid( city ) );
  ret->setPathway( lady->getPathway() );
  ret->setIJ( lady->getIJ() );
  ret->_getPathway().rbegin();
  ret->onMidTile();
  ret->drop();

  return ret;
}

MarketKid::MarketKid( CityPtr city )
  : Walker( city ), _d( new Impl )
{
  _d->delay = 0;
  _d->birthTime = 0;
  _d->basket._maxQty = 100;
  _setAnimation( gfx::marketkid );
  _setType( walker::marketKid );

  setName( NameGenerator::rand( NameGenerator::male ) );
}

void MarketKid::setDelay( int delay )
{
  _d->delay = delay;
}

void MarketKid::send2City( MarketPtr destination )
{
  if( destination.isValid() )
  {
    _d->marketPos = destination->getTilePos();
    _getCity()->addWalker( this );
  }
  else
  {
    deleteLater();
  }
}

void MarketKid::timeStep( const unsigned long time )
{
  if( _d->birthTime == 0 )
  {
    _d->birthTime = time;
  }

  if( time - _d->birthTime < _d->delay )
  {
    return;
  }

  Walker::timeStep( time );
}

void MarketKid::onDestination()
{
  Walker::onDestination();

  deleteLater();

  CityHelper cityh( _getCity() );
  MarketPtr market = cityh.find<Market>( building::market, _d->marketPos );
  if( market.isValid() )
  {
    market->getGoodStore().store( _d->basket, _d->basket._currentQty );
  }
}

void MarketKid::die()
{
  Walker::die();

  Corpse::create( _getCity(), getIJ(), ResourceGroup::carts, 465, 472 );
}

GoodStock& MarketKid::getBasket()
{
  return _d->basket;
}
