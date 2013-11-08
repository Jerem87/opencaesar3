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

#include "layerwater.hpp"
#include "building/constants.hpp"
#include "game/resourcegroup.hpp"
#include "building/house.hpp"
#include "game/house_level.hpp"
#include "layerconstants.hpp"

using namespace constants;

int LayerWater::getType() const
{
  return citylayer::water;
}

std::set<int> LayerWater::getVisibleWalkers() const
{
  return std::set<int>();
}

void LayerWater::drawTile(GfxEngine& engine, Tile& tile, Point offset)
{
  Point screenPos = tile.getXY() + offset;

  tile.setWasDrawn();

  bool needDrawAnimations = false;
  Size areaSize(1);

  if( tile.getOverlay().isNull() )
  {
    //draw background
    engine.drawPicture( tile.getPicture(), screenPos );
  }
  else
  {
    TileOverlayPtr overlay = tile.getOverlay();
    Picture pic;
    switch( overlay->getType() )
    {
      //water buildings
    case construction::road:
    case construction::B_PLAZA:
    case building::B_RESERVOIR:
    case building::B_FOUNTAIN:
    case building::B_WELL:
    case building::B_AQUEDUCT:
      pic = tile.getPicture();
      needDrawAnimations = true;
      areaSize = overlay->getSize();
    break;

    default:
    {
      int tileNumber = 0;
      bool haveWater = tile.getWaterService( WTR_FONTAIN ) > 0 || tile.getWaterService( WTR_WELL ) > 0;
      if ( overlay->getType() == building::house )
      {
        HousePtr h = overlay.as<House>();
        tileNumber = OverlayPic::inHouse;
        haveWater = haveWater || h->hasServiceAccess(Service::fontain) || h->hasServiceAccess(Service::well);
      }
      tileNumber += (haveWater ? OverlayPic::haveWater : 0);
      tileNumber += tile.getWaterService( WTR_RESERVOIR ) > 0 ? OverlayPic::reservoirRange : 0;

      CityHelper helper( _city );
      drawArea( engine, helper.getArea( overlay ), offset, ResourceGroup::waterOverlay, OverlayPic::base + tileNumber );

      pic = Picture::getInvalid();
      areaSize = 0;
      needDrawAnimations = false;
    }
    break;
    }

    if ( pic.isValid() )
    {
      engine.drawPicture( pic, screenPos );
    }

    if( needDrawAnimations )
    {
      _renderer->registerTileForRendering( tile );
    }
  }

  if( !needDrawAnimations && ( tile.isWalkable(true) || tile.getFlag( Tile::tlBuilding ) ) )
  {
    Tilemap& tilemap = _city->getTilemap();
    TilemapArea area = tilemap.getArea( tile.getIJ(), areaSize );

    foreach( Tile* rtile, area )
    {
      int reservoirWater = rtile->getWaterService( WTR_RESERVOIR );
      int fontainWater = rtile->getWaterService( WTR_FONTAIN );

      if( (reservoirWater + fontainWater > 0) && ! rtile->getFlag( Tile::tlWater ) && rtile->getOverlay().isNull() )
      {
        int picIndex = reservoirWater ? OverlayPic::reservoirRange : 0;
        picIndex |= fontainWater > 0 ? OverlayPic::haveWater : 0;
        picIndex |= OverlayPic::skipLeftBorder | OverlayPic::skipRightBorder;
        engine.drawPicture( Picture::load( ResourceGroup::waterOverlay, picIndex + OverlayPic::base ), rtile->getXY() + offset );
      }
    }
  }

}

LayerPtr LayerWater::create(CityRenderer* renderer, CityPtr city)
{
  LayerWater* l = new LayerWater();
  l->_renderer = renderer;
  l->_city = city;

  LayerPtr ret( l );
  ret->drop();

  return ret;
}
