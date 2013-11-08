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

#include "advisor_entertainment_window.hpp"
#include "gfx/decorator.hpp"
#include "core/gettext.hpp"
#include "texturedbutton.hpp"
#include "label.hpp"
#include "game/resourcegroup.hpp"
#include "core/stringhelper.hpp"
#include "gfx/engine.hpp"
#include "core/gettext.hpp"
#include "game/enums.hpp"
#include "game/city.hpp"
#include "core/foreach.hpp"
#include "building/house.hpp"
#include "festival_planing_window.hpp"
#include "game/settings.hpp"
#include "game/house_level.hpp"
#include "building/entertainment.hpp"
#include "game/cityservice_festival.hpp"
#include "game/divinity.hpp"
#include "game/gamedate.hpp"
#include "core/logger.hpp"
#include "building/constants.hpp"

using namespace constants;

namespace gui
{

struct InfrastructureInfo
{
  int buildingCount;
  int partlyWork;
  int buildingWork;
  int buildingShow;
  int peoplesServed;
};

class EntertainmentInfoLabel : public Label
{
public:
  EntertainmentInfoLabel( Widget* parent, const Rect& rect,
                          const TileOverlay::Type service, InfrastructureInfo info  )
    : Label( parent, rect )
  {
    _service = service;
    _info = info;

    setFont( Font::create( FONT_1_WHITE ) );
  }

  const InfrastructureInfo& getInfo() const
  {
    return _info;
  }

  virtual void _updateTexture( GfxEngine& painter )
  {
    Label::_updateTexture( painter );

    std::string buildingStr, peoplesStr;
    switch( _service )
    {
    case building::theater: buildingStr = _("##theaters##"); peoplesStr = _("##peoples##"); break;
    case building::amphitheater: buildingStr = _("##amphitheatres##"); peoplesStr = _("##peoples##"); break;
    case building::colloseum: buildingStr = _("##colloseum##"); peoplesStr = _("##peoples##"); break;
    case building::hippodrome: buildingStr = _("##hippodromes##"); peoplesStr = "-"; break;
    default:
    break;
    }

    PictureRef& texture = getTextPicture();
    Font font = getFont();
    font.draw( *texture, StringHelper::format( 0xff, "%d %s", _info.buildingCount, buildingStr.c_str() ), 0, 0 );
    font.draw( *texture, StringHelper::format( 0xff, "%d", _info.buildingWork ), 165, 0 );
    font.draw( *texture, StringHelper::format( 0xff, "%d", _info.buildingShow ), 245, 0 );
    font.draw( *texture, StringHelper::format( 0xff, "%d %s", _info.peoplesServed, peoplesStr.c_str() ), 305, 0 );
  }

private:
  TileOverlay::Type _service;
  InfrastructureInfo _info;
};

class AdvisorEntertainmentWindow::Impl
{
public:
  CityPtr city;

  EntertainmentInfoLabel* lbTheatresInfo;
  EntertainmentInfoLabel* lbAmphitheatresInfo;
  EntertainmentInfoLabel* lbColisseumInfo;
  EntertainmentInfoLabel* lbHippodromeInfo;
  Label* lbBlackframe;
  Label* lbTroubleInfo;
  PushButton* btnNewFestival;
  Label* lbInfoAboutLastFestival;
  TexturedButton* btnHelp;
  Label* lbMonthFromLastFestival;
  SmartPtr<CityServiceFestival> srvc;

  InfrastructureInfo getInfo( CityPtr city, const TileOverlay::Type service );
  void assignFestival(int divinityType, int festSize);
  void updateInfo();
  void updateFestivalInfo();
};


AdvisorEntertainmentWindow::AdvisorEntertainmentWindow( CityPtr city, Widget* parent, int id ) 
: Widget( parent, id, Rect( 0, 0, 1, 1 ) ), _d( new Impl )
{
  _d->city = city;
  _d->srvc = city->findService( CityServiceFestival::getDefaultName() ).as<CityServiceFestival>();

  setupUI( GameSettings::rcpath( "/gui/entertainmentadv.gui" ) );

  setPosition( Point( (parent->getWidth() - getWidth() )/2, parent->getHeight() / 2 - 242 ) );

  _d->lbBlackframe = findChild<Label*>( "lbBlackframe", true );
  _d->lbTroubleInfo = findChild<Label*>( "lbTroubleInfo", true );
  _d->btnHelp = findChild<TexturedButton*>( "btnHelp", true );
  _d->btnNewFestival = findChild<PushButton*>( "btnNewFestival", true );
  _d->lbMonthFromLastFestival = findChild<Label*>( "lbMonthFromLastFestival", true );
  _d->lbInfoAboutLastFestival = findChild<Label*>( "lbInfoAboutLastFestival", true );

  Point startPoint( 2, 2 );
  Size labelSize( 550, 20 );
  InfrastructureInfo info;
  info = _d->getInfo( city, building::theater );
  _d->lbTheatresInfo = new EntertainmentInfoLabel( _d->lbBlackframe, Rect( startPoint, labelSize ), building::theater, info );

  info = _d->getInfo( city, building::amphitheater );
  _d->lbAmphitheatresInfo = new EntertainmentInfoLabel( _d->lbBlackframe, Rect( startPoint + Point( 0, 20), labelSize), building::amphitheater,
                                                        info );
  info = _d->getInfo( city, building::colloseum );
  _d->lbColisseumInfo = new EntertainmentInfoLabel( _d->lbBlackframe, Rect( startPoint + Point( 0, 40), labelSize), building::colloseum, info );

  info = _d->getInfo( city, building::hippodrome );
  _d->lbHippodromeInfo = new EntertainmentInfoLabel( _d->lbBlackframe, Rect( startPoint + Point( 0, 60), labelSize), building::hippodrome, info );

  CONNECT( _d->btnNewFestival, onClicked(), this, AdvisorEntertainmentWindow::_showFestivalWindow );

  _d->updateInfo();
  _d->updateFestivalInfo();
}

void AdvisorEntertainmentWindow::draw( GfxEngine& painter )
{
  if( !isVisible() )
    return;

  Widget::draw( painter );
}

void AdvisorEntertainmentWindow::_showFestivalWindow()
{
  FestivalPlaningWindow* wnd = FestivalPlaningWindow::create( this, _d->city, -1 );
  CONNECT( wnd, onFestivalAssign(), _d.data(), Impl::assignFestival );
}

InfrastructureInfo AdvisorEntertainmentWindow::Impl::getInfo(CityPtr city, const TileOverlay::Type service)
{
  CityHelper helper( city );

  InfrastructureInfo ret;

  ret.buildingWork = 0;
  ret.peoplesServed = 0;
  ret.buildingShow = 0;
  ret.buildingCount = 0;
  ret.partlyWork = 0;

  ServiceBuildingList servBuildings = helper.find<ServiceBuilding>( service );
  foreach( ServiceBuildingPtr building, servBuildings )
  {
    if( building->getWorkers() > 0 )
    {
      ret.buildingWork++;

      int maxServing = 0;
      switch( service )
      {
      case building::theater: maxServing = 500; break;
      case building::amphitheater: maxServing = 800; break;
      case building::colloseum: maxServing = 1500; break;
      default:
      break;
      }

      ret.peoplesServed += maxServing * building->getWorkers() / building->getMaxWorkers();
    }
    ret.buildingCount++;
    ret.partlyWork += (building->getWorkers() != building->getMaxWorkers() ? 1 : 0);
  }

  return ret;
}

void AdvisorEntertainmentWindow::Impl::assignFestival(int divinityType, int festSize)
{
  if( srvc.isValid() )
  {
    srvc->assignFestival( (RomeDivinityType)divinityType, festSize );
  }
}

void AdvisorEntertainmentWindow::Impl::updateInfo()
{ 
  StringArray troubles;
  if( !lbTroubleInfo )
    return;

  const InfrastructureInfo& thInfo = lbTheatresInfo->getInfo();
  const InfrastructureInfo& amthInfo = lbAmphitheatresInfo->getInfo();
  const InfrastructureInfo& clsInfo = lbColisseumInfo->getInfo();
  //const InfrastructureInfo& hpdInfo = lbHippodromeInfo->getInfo();

  CityHelper helper( city );
  int theatersNeed = 0, amptNeed = 0, clsNeed = 0, hpdNeed = 0;
  int theatersServed = 0, amptServed = 0, clsServed = 0, hpdServed = 0;
  int nextLevel = 0;

  HouseList houses = helper.find<House>( building::house );
  foreach( HousePtr house, houses )
  {
    int habitants = house->getHabitants().count( CitizenGroup::mature );

    const HouseLevelSpec& lspec = house->getSpec();

    if( house->isEntertainmentNeed( Service::theater ) )
    {
      theatersNeed +=  habitants;
      theatersServed += (house->hasServiceAccess( Service::theater ) ? habitants : 0);
    }

    if(house->isEntertainmentNeed( Service::amphitheater ))
    {
      amptNeed +=  habitants;
      amptServed += (house->hasServiceAccess( Service::amphitheater ) ? habitants : 0 );
    }

    if(house->isEntertainmentNeed( Service::colloseum ))
    {
      clsNeed += habitants;
      clsServed += (house->hasServiceAccess( Service::colloseum) ? habitants : 0);
    }

    if( house->isEntertainmentNeed( Service::hippodrome ) )
    {
      hpdNeed += habitants;
      hpdServed += (house->hasServiceAccess( Service::hippodrome) ? habitants : 0);
    }

    nextLevel += ((lspec.computeEntertainmentLevel( house ) - lspec.getMinEntertainmentLevel()) < 0 ? 1 : 0);
  }

  int allNeed = theatersNeed + amptNeed + clsNeed + hpdNeed;
  int allServed = theatersServed + amptServed + clsServed + hpdServed;

  int entertCoverage = (allNeed > 0 ? (allServed * 100 / allNeed) : 0);

  if( entertCoverage > 80 && entertCoverage <= 100 )     { troubles.push_back( "##entertainment_80_100##" ); }
  else if( entertCoverage > 50 && entertCoverage <= 80 ) { troubles.push_back( "##entertainment_50_80##" ); }
  else if( allNeed > 0 && entertCoverage <= 50 )         { troubles.push_back( "##entertainment_less_50##" ); }

  if( thInfo.partlyWork > 0 ) { troubles.push_back( "" ); }
  if( amthInfo.partlyWork > 0 ) { troubles.push_back( "##some_amphitheaters_no_actors##" ); }
  if( clsInfo.partlyWork > 0 ) { troubles.push_back( "##small_colloseum_show##" ); }

  HippodromeList hippodromes = helper.find<Hippodrome>( building::hippodrome );
  foreach( HippodromePtr h, hippodromes )
  {
    if( h->evaluateTrainee( walker::charioter ) == 100 ) { troubles.push_back( "##no_chariots##" ); }
  }

  if( nextLevel > 0 )
  {
    troubles.push_back( "##entertainment_need_for_upgrade##" );
  }

  if( theatersNeed == 0 )
  {
    troubles.push_back( "##entertainment_not_need##" );
  }

  if( troubles.empty() )
  {
    troubles.push_back( "##entertainment_full##" );
  }

  lbTroubleInfo->setText( _( troubles.at( rand() % troubles.size() ).c_str() ) );
}

void AdvisorEntertainmentWindow::Impl::updateFestivalInfo()
{
  if( srvc.isValid() )
  {
    int monthFromLastFestival = srvc->getLastFestivalDate().getMonthToDate( GameDate::current() );
    std::string text = StringHelper::format( 0xff, "%d %s", monthFromLastFestival, _("##month_from_last_festival##") );

    if( lbMonthFromLastFestival ) { lbMonthFromLastFestival->setText( text ); }

    bool prepare2Festival = srvc->getNextFestivalDate() >= GameDate::current();
    btnNewFestival->setText( prepare2Festival ? _("##prepare_to_festival##") : _("##new_festival##") );
    btnNewFestival->setEnabled( !prepare2Festival );

    text = StringHelper::format( 0xff, "##more_%d_month_from_festival##", math::clamp( monthFromLastFestival / 4 * 4, 0, 24) );
    if( lbInfoAboutLastFestival ) { lbInfoAboutLastFestival->setText( _( text.c_str() ) ); }
  }
}

}//end namespace gui
