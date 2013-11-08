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

#include "pottery.hpp"
#include "gfx/picture.hpp"
#include "game/resourcegroup.hpp"
#include "game/city.hpp"
#include "core/gettext.hpp"
#include "constants.hpp"

using namespace constants;

Pottery::Pottery() : Factory(Good::clay, Good::pottery, building::pottery, Size(2))
{
  _getAnimation().load(ResourceGroup::commerce, 133, 7);
  _getAnimation().setDelay( 3 );
  _getFgPictures().resize(2);
}

bool Pottery::canBuild(CityPtr city, const TilePos& pos) const
{
  bool ret = Factory::canBuild( city, pos );

  CityHelper helper( city );
  bool haveClaypit = !helper.find<Building>( building::clayPit ).empty();

  const_cast< Pottery* >( this )->_setError( haveClaypit ? "" : _("##need_clay_for_work##") );

  return ret;
}

void Pottery::timeStep( const unsigned long time )
{
  Factory::timeStep( time );
}
