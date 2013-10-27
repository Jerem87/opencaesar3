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

#include "event.hpp"
#include "game/tileoverlay_factory.hpp"
#include "game/city.hpp"
#include "core/gettext.hpp"
#include "building/metadata.hpp"
#include "game/cityfunds.hpp"
#include "gui/environment.hpp"
#include "gui/info_box.hpp"
#include "game/tilemap.hpp"
#include "game/game.hpp"
#include "dispatcher.hpp"
#include "core/stringhelper.hpp"
#include "gui/label.hpp"
#include "gui/empiremap_window.hpp"
#include "game/empire.hpp"
#include "game/resourcegroup.hpp"
#include "gui/advisors_window.hpp"
#include "game/trade_options.hpp"
#include "gui/message_stack_widget.hpp"
#include "game/settings.hpp"

namespace events
{

static const int windowGamePausedId = StringHelper::hash( "gamepause" );

GameEventPtr DisasterEvent::create( const TilePos& pos, Type type )
{
  DisasterEvent* event = new DisasterEvent();
  event->_pos = pos;
  event->_type = type;

  GameEventPtr ret( event );
  ret->drop();

  return ret;
}

void DisasterEvent::exec( Game& game )
{
  Tilemap& tmap = game.getCity()->getTilemap();
  Tile& tile = tmap.at( _pos );
  TilePos rPos = _pos;

  if( tile.getFlag( Tile::isDestructible ) )
  {
    Size size( 1 );

    TileOverlayPtr overlay = tile.getOverlay();
    if( overlay.isValid() )
    {
      overlay->deleteLater();
      size = overlay->getSize();
      rPos = overlay->getTile().getIJ();
    }

    //bool deleteRoad = false;

    TilemapArea clearedTiles = tmap.getArea( rPos, size );
    foreach( Tile* tile, clearedTiles )
    {
      TileOverlayType dstr2constr[] = { B_BURNING_RUINS, B_COLLAPSED_RUINS, B_PLAGUE_RUINS };
      bool canCreate = TileOverlayFactory::getInstance().canCreate( dstr2constr[_type] );
      if( canCreate )
      {
        Dispatcher::append( BuildEvent::create( tile->getIJ(), dstr2constr[_type] ) );
      }
    }

    std::string dstr2string[] = { _("##alarm_fire_in_city##"), _("##alarm_building_collapsed##"),
                                  _("##alarm_plague_in_city##") };
    game.getCity()->onDisasterEvent().emit( _pos, dstr2string[_type] );
  }
}

GameEventPtr BuildEvent::create( const TilePos& pos, const TileOverlayType type )
{
  return create( pos, TileOverlayFactory::getInstance().create( type ) );
}

GameEventPtr BuildEvent::create(const TilePos& pos, TileOverlayPtr overlay)
{
  BuildEvent* ev = new BuildEvent();
  ev->_pos = pos;
  ev->_overlay = overlay;

  GameEventPtr ret( ev );
  ret->drop();

  return ret;
}

void BuildEvent::exec( Game& game )
{
  const BuildingData& buildingData = BuildingDataHolder::instance().getData( _overlay->getType() );
  if( _overlay.isValid() )
  {
    _overlay->build( game.getCity(), _pos );

    ConstructionPtr construction = _overlay.as<Construction>();
    if( construction != 0 )
    {
      CityHelper helper( game.getCity() );
      helper.updateDesirability( construction, true );

      game.getCity()->addOverlay( _overlay );
      game.getCity()->getFunds().resolveIssue( FundIssue( CityFunds::buildConstruction, -buildingData.getCost() ) );

      if( construction->isNeedRoadAccess() && construction->getAccessRoads().empty() )
      {
        game.getCity()->onWarningMessage().emit( "##building_need_road_access##" );
      }
    }
  }
}

GameEventPtr ClearLandEvent::create(const TilePos& pos)
{
  ClearLandEvent* ev = new ClearLandEvent();
  ev->_pos = pos;

  GameEventPtr ret( ev );
  ret->drop();

  return ret;
}

void ClearLandEvent::exec( Game& game )
{
  Tilemap& tmap = game.getCity()->getTilemap();

  Tile& cursorTile = tmap.at( _pos );

  if( cursorTile.getFlag( Tile::isDestructible ) )
  {
    Size size( 1 );
    TilePos rPos = _pos;

    TileOverlayPtr overlay = cursorTile.getOverlay();

    bool deleteRoad = false;

    if (cursorTile.getFlag( Tile::tlRoad )) deleteRoad = true;

    if ( overlay.isValid() )
    {
      size = overlay->getSize();
      rPos = overlay->getTile().getIJ();
      overlay->deleteLater();
    }

    TilemapArea clearedTiles = tmap.getArea( rPos, size );
    foreach( Tile* tile, clearedTiles )
    {
      tile->setMasterTile( NULL );
      tile->setFlag( Tile::tlTree, false);
      tile->setFlag( Tile::tlBuilding, false);
      tile->setFlag( Tile::tlRoad, false);
      tile->setFlag( Tile::tlGarden, false);
      tile->setOverlay( NULL );

      deleteRoad |= tile->getFlag( Tile::tlRoad );

      if( tile->getFlag( Tile::tlMeadow ) )
      {
        tile->setPicture( TileHelper::convId2PicName( tile->getOriginalImgId() ) );
      }
      else
      {
        // choose a random landscape picture:
        // flat land1a 2-9;
        // wheat: land1a 18-29;
        // green_something: land1a 62-119;  => 58
        // green_flat: land1a 232-289; => 58

        // choose a random background image, green_something 62-119 or green_flat 232-240
         // 30% => choose green_sth 62-119
        // 70% => choose green_flat 232-289
        int startOffset  = ( (rand() % 10 > 6) ? 62 : 232 );
        int imgId = rand() % 58;

        tile->setPicture( ResourceGroup::land1a, startOffset + imgId );
      }
    }

    // recompute roads;
    // there is problem that we NEED to recompute all roads map for all buildings
    // because MaxDistance2Road can be any number
    //
    if( deleteRoad )
    {
      game.getCity()->updateRoads();
    }
  }
}

GameEventPtr ShowInfoboxEvent::create( const std::string& title, const std::string& text )
{
  ShowInfoboxEvent* ev = new ShowInfoboxEvent();
  ev->_title = title;
  ev->_text = text;

  GameEventPtr ret( ev );
  ret->drop();

  return ret;
}

void ShowInfoboxEvent::exec( Game& game )
{
  gui::InfoBoxText* msgWnd = new gui::InfoBoxText( game.getGui()->getRootWidget(), _title, _text );
  msgWnd->show();
}


GameEventPtr TogglePause::create()
{
  GameEventPtr ret( new TogglePause() );
  ret->drop();
  return ret;
}

void TogglePause::exec(Game& game)
{
  gui::Widget* rootWidget = game.getGui()->getRootWidget();
  gui::Label* wdg = safety_cast< gui::Label* >( rootWidget->findChild( windowGamePausedId ) );
  game.setPaused( !game.isPaused() );

  if( game.isPaused()  )
  {
    if( !wdg )
    {
      Size scrSize = rootWidget->getSize();
      wdg = new gui::Label( rootWidget, Rect( Point( (scrSize.getWidth() - 450)/2, 40 ), Size( 450, 50 ) ),
                            _("##game_is_paused##"), false, gui::Label::bgWhiteFrame, windowGamePausedId );
      wdg->setTextAlignment( alignCenter, alignCenter );
    }
  }
  else
  {
    if( wdg )
    {
      wdg->deleteLater();
    }
  }
}


GameEventPtr ChangeSpeed::create(int value)
{
  ChangeSpeed* ev = new ChangeSpeed();
  ev->_value = value;
  GameEventPtr ret( ev );
  ret->drop();
  return ret;
}

void ChangeSpeed::exec(Game& game)
{
  game.changeTimeMultiplier( _value );
}


GameEventPtr FundIssueEvent::create(int type, int value)
{
  FundIssueEvent* ev = new FundIssueEvent();
  ev->_value = value;
  ev->_type = type;
  GameEventPtr ret( ev );
  ret->drop();
  return ret;
}

GameEventPtr FundIssueEvent::import(Good::Type good, int qty)
{
  FundIssueEvent* ev = new FundIssueEvent();
  ev->_gtype = good;
  ev->_qty = qty;
  ev->_type = CityFunds::importGoods;
  GameEventPtr ret( ev );
  ret->drop();
  return ret;
}

GameEventPtr FundIssueEvent::exportg(Good::Type good, int qty)
{
  FundIssueEvent* ev = new FundIssueEvent();
  ev->_gtype = good;
  ev->_qty = qty;
  ev->_type = CityFunds::exportGoods;
  GameEventPtr ret( ev );
  ret->drop();
  return ret;
}

void FundIssueEvent::exec(Game& game)
{
  if( _type == CityFunds::importGoods )
  {
    int price = game.getCity()->getTradeOptions().getSellPrice( _gtype );
    _value = -price * _qty / 100;
  }
  else if( _type == CityFunds::exportGoods )
  {
    int price = game.getCity()->getTradeOptions().getBuyPrice( _gtype );
    _value = price * _qty / 100;
  }

  game.getCity()->getFunds().resolveIssue( FundIssue( _type, _value ) );
}


GameEventPtr ShowEmpireMapWindow::create(bool show)
{
  ShowEmpireMapWindow* ev = new ShowEmpireMapWindow();
  ev->_show = show;
  GameEventPtr ret( ev );
  ret->drop();
  return ret;
}

void ShowEmpireMapWindow::exec(Game& game)
{
  List<gui::EmpireMapWindow*> wndList = game.getGui()->getRootWidget()->findChildren<gui::EmpireMapWindow*>();

  if( _show )
  {
    if( !wndList.empty() )
    {
      wndList.front()->bringToFront();
    }
    else
    {
      gui::EmpireMapWindow::create( game.getEmpire(), game.getCity(), game.getGui()->getRootWidget(), -1 );
    }
  }
  else
  {
    if( !wndList.empty() )
    {
      wndList.front()->deleteLater();
    }
  }
}


GameEventPtr ShowAdvisorWindow::create(bool show, int advisor)
{
  ShowAdvisorWindow* ev = new ShowAdvisorWindow();
  ev->_show = show;
  ev->_advisor = advisor;
  GameEventPtr ret( ev );
  ret->drop();
  return ret;
}

void ShowAdvisorWindow::exec(Game& game)
{
  List<gui::AdvisorsWindow*> wndList = game.getGui()->getRootWidget()->findChildren<gui::AdvisorsWindow*>();

  if( _show )
  {
    if( !wndList.empty() )
    {
      wndList.front()->bringToFront();
      wndList.front()->showAdvisor( (AdvisorType)_advisor );
    }
    else
    {
      gui::AdvisorsWindow::create( game.getGui()->getRootWidget(), -1, (AdvisorType)_advisor, game.getCity() );
    }
  }
  else
  {
    if( !wndList.empty() )
    {
      wndList.front()->deleteLater();
    }
  }
}

GameEventPtr WarningMessageEvent::create(const std::string& text)
{
  WarningMessageEvent* ev = new WarningMessageEvent();
  ev->_text = text;
  GameEventPtr ret( ev );
  ret->drop();
  return ret;
}

void WarningMessageEvent::exec(Game& game)
{
  gui::WindowMessageStack* window = safety_cast<gui::WindowMessageStack*>(
                                      game.getGui()->getRootWidget()->findChild( gui::WindowMessageStack::defaultID ) );

  if( window )
  {
    window->addMessage( _text );
  }
}

void events::GameEvent::dispatch()
{
  Dispatcher::append( this );
}

} //end namespace events
