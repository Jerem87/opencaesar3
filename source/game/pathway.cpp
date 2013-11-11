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

#include "pathway.hpp"
#include "gfx/tile.hpp"
#include "tilemap.hpp"
#include "core/direction.hpp"
#include "core/logger.hpp"

using namespace  constants;

bool operator<(const Pathway &v1, const Pathway &v2)
{
  if (v1.getLength()!=v2.getLength())
  {
    return v1.getLength() < v2.getLength();
  }

  // compare memory address
  return (&v1 < &v2);
}

class Pathway::Impl
{
public:
  TilePos destination;
  bool isReverse;

  typedef std::vector<constants::Direction> Directions;
  Directions directionList;
  Directions::iterator directionIt;
  Directions::reverse_iterator directionIt_reverse;
  ConstTilemapTiles tileList;
};

Pathway::Pathway() : _d( new Impl )
{
  _origin = NULL;
  _d->destination = TilePos( 0, 0 );
  _d->isReverse = false;
}

Pathway::~Pathway()
{

}

Pathway::Pathway(const Pathway &copy) : _d( new Impl )
{
  *this = copy;
}

void Pathway::init( const Tilemap &tilemap, const Tile &origin)
{
  _tilemap = &tilemap;
  _origin = &origin;
  _d->destination = origin.getIJ();
  _d->directionList.clear();
  _d->directionIt = _d->directionList.begin();
  _d->directionIt_reverse = _d->directionList.rbegin();
  _d->tileList.clear();
  _d->tileList.push_back(&origin);
}

int Pathway::getLength() const
{
  // TODO: various lands have various travel time (road easier to travel than open country)
  return _d->directionList.size();
}

const Tile& Pathway::getOrigin() const
{
  return *_origin;
}

const Tile& Pathway::getDestination() const
{
  const Tile& res = _tilemap->at( _d->destination );
  return res;
}

bool Pathway::isReverse() const
{
  return _d->isReverse;
}

void Pathway::begin()
{
  _d->directionIt = _d->directionList.begin();
  _d->isReverse = false;
}

void Pathway::rbegin()
{
  _d->directionIt_reverse = _d->directionList.rbegin();
  _d->isReverse = true;
}

void Pathway::toggleDirection()
{
  if( _d->isReverse )
  {
    _d->isReverse = false;
    _d->directionIt = _d->directionIt_reverse.base();
  }
  else
  {
    _d->isReverse = true;
    _d->directionIt_reverse = Impl::Directions::reverse_iterator( _d->directionIt );
  }
}

constants::Direction Pathway::getNextDirection()
{
  Direction res = noneDirection;
  if (_d->isReverse)
  {
    if (_d->directionIt_reverse == _d->directionList.rend())
    {
      // end of path!
      return noneDirection;
    }
    int direction = (int) *_d->directionIt_reverse;
    if( direction != (int) noneDirection )
    {
      if (direction + 4 < (int) countDirection)
      {
        res = (Direction) (direction+4);
      }
      else
      {
        res = (Direction) (direction-4);
      }
    }
    _d->directionIt_reverse ++;
  }
  else
  {
    if (_d->directionIt == _d->directionList.end())
    {
      // end of path!
      return noneDirection;
    }
    res = *_d->directionIt;
    _d->directionIt ++;
  }

  return res;
}

bool Pathway::isDestination() const
{
  bool res;
  if (_d->isReverse)
  {
#if defined(OC3_PLATFORM_WIN)
    Impl::Directions::const_reverse_iterator convItReverse = _d->directionIt_reverse;
    res = (convItReverse == _d->directionList.rend());
#elif defined(OC3_PLATFORM_UNIX)
    res = (_d->directionIt_reverse == _d->directionList.rend());
#endif //OC3_PLATFORM_UNIX
  }
  else
  {
    res = (_d->directionIt == _d->directionList.end());
  }

  return res;
}

void Pathway::setNextDirection(Direction direction)
{
  switch (direction)
  {
  case north      : _d->destination += TilePos( 0, 1 );  break;
  case northEast  : _d->destination += TilePos( 1, 1 );  break;
  case east       : _d->destination += TilePos( 1, 0 );  break;
  case southEast  : _d->destination += TilePos( 1, -1 ); break;
  case south      : _d->destination += TilePos( 0, -1 ); break;
  case southWest  : _d->destination += TilePos( -1, -1 );break;
  case west       : _d->destination += TilePos( -1, 0 ); break;
  case northWest  : _d->destination += TilePos( -1, 1 ); break;
  default:
    _d->destination += TilePos( 0, 1 );  break;
    Logger::warning( "Unexpected Direction:%d", direction);
  break;
  }


  if( !_tilemap->isInside( TilePos( _d->destination ) ) )
  {
    Logger::warning( "Destination[%d, %d] out of map", _d->destination.getI(), _d->destination.getJ() );
  }
  else
  {
    _d->tileList.push_back( &_tilemap->at( _d->destination ) );
    _d->directionList.push_back(direction);
  }
}

void Pathway::setNextTile( const Tile& tile )
{
  int dI = tile.getI() - _d->destination.getI();
  int dJ = tile.getJ() - _d->destination.getJ();

  Direction direction;

  if (dI==0 && dJ==0) {  direction = noneDirection; }
  else if (dI==0 && dJ==1) { direction = north; }
  else if (dI==1 && dJ==1) { direction = northEast; }
  else if (dI==1 && dJ==0) { direction = east; }
  else if (dI==1 && dJ==-1){ direction = southEast; }
  else if (dI==0 && dJ==-1){ direction = south; }
  else if (dI==-1 && dJ==-1){ direction = southWest;}
  else if (dI==-1 && dJ==0) {direction = west;}
  else if (dI==-1 && dJ==1){direction = northWest; }
  else
  {
    Logger::warning( "Destination[%d, %d] out of map", dI, dJ );
    direction = noneDirection;
  }

  setNextDirection(direction);
}

bool Pathway::contains(Tile &tile)
{
  // search in reverse direction, because usually the last tile matches
  bool res = false;
  for( ConstTilemapTiles::reverse_iterator itTile = _d->tileList.rbegin();
       itTile != _d->tileList.rend(); ++itTile)
  {
    if (*itTile == &tile)
    {
      res = true;
      break;
    }
  }

  return res;
}

ConstTilemapTiles& Pathway::getAllTiles()
{
  return _d->tileList;
}

void Pathway::prettyPrint() const
{
  if (_origin == NULL)
  {
    Logger::warning( "pathWay is NULL" );
  }
  else
  {
    Logger::warning( "pathWay from [%d,%d] to [%d,%d]",
                     _origin->getI(), _origin->getJ(), _d->destination.getI(), _d->destination.getJ() );

    std::string strDir = "";
    for( Impl::Directions::const_iterator itDir = _d->directionList.begin();
         itDir != _d->directionList.end(); ++itDir)
    {
      Direction direction = *itDir;

      switch (direction)
      {
      case north: strDir += "N";  break;
      case northEast: strDir += "NE"; break;
      case east: strDir += "E"; break;
      case southEast: strDir += "SE"; break;
      case south: strDir += "S";   break;
      case southWest: strDir += "SW"; break;
      case west: strDir += "W";  break;
      case northWest: strDir += "NW"; break;
      default:
        _OC3_DEBUG_BREAK_IF( "Unexpected Direction:" || direction);
      break;
      }

      strDir += " ";
    }

    Logger::warning( strDir.c_str() );
  }
}

VariantMap Pathway::save() const
{
  VariantMap stream;
  if( getLength() == 0 ) //not save empty way
  {
    return VariantMap();
  }

  stream[ "startPos" ] = _origin->getIJ();
  stream[ "stopPos" ] = _d->destination;

  VariantList directions;
  for( Impl::Directions::const_iterator itDir = _d->directionList.begin();
       itDir != _d->directionList.end(); ++itDir)
  {
    directions.push_back( (int)*itDir );
  }

  stream[ "directions" ] = directions;
  stream[ "reverse" ] = _d->isReverse;
  stream[ "step" ] = getStep();

  return stream;
}

bool Pathway::isValid() const
{
  return getLength() != 0;
}

void Pathway::load( const VariantMap& stream )
{
  if( stream.size() == 0 )
  {
    return;
  }

  _origin = &_tilemap->at( stream.get( "startPos" ).toTilePos() );
  _d->destination = _origin->getIJ(); //stream.get( "stopPos" ).toTilePos();
  VariantList directions = stream.get( "directions" ).toList();
  for( VariantList::iterator it = directions.begin(); it != directions.end(); it++ )
  {
    Direction dir = (Direction)(*it).toInt();
    setNextDirection( dir );
  }

  _d->isReverse = stream.get( "reverse" ).toBool();
  int off = stream.get( "step" ).toInt();
  _d->directionIt = _d->directionList.begin();
  _d->directionIt_reverse = _d->directionList.rbegin();
  std::advance(_d->directionIt_reverse, off);
  std::advance(_d->directionIt, off);
}

Pathway& Pathway::operator=( const Pathway& other )
{
  _tilemap             = other._tilemap;
  _origin              = other._origin;
  _d->destination      = other._d->destination;
  _d->directionList       = other._d->directionList;
  _d->directionIt         = _d->directionList.begin();
  _d->directionIt_reverse = _d->directionList.rbegin();
  _d->tileList            = other._d->tileList;

  return *this;
}

unsigned int Pathway::getStep() const
{
  if(_d->isReverse)
  {
    size_t pos = std::distance<Impl::Directions::const_reverse_iterator>(_d->directionList.rbegin(), _d->directionIt_reverse);
    return static_cast<unsigned int>(pos);
  }
  else
  {
    size_t pos = std::distance<Impl::Directions::const_iterator>(_d->directionList.begin(), _d->directionIt);
    return static_cast<unsigned int>(pos);
  }
}
