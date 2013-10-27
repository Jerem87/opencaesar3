#include "modal_widget.hpp"
#include "core/safetycast.hpp"
#include "core/event.hpp"
#include "core/foreach.hpp"
#include "gfx/engine.hpp"
#include "environment.hpp"

namespace gui
{

//! constructor
ModalScreen::ModalScreen( Widget* parent, int id)
: Widget( parent, id, Rect(0, 0, parent->getWidth(), parent->getHeight() ) ),
	_mouseDownTime(0)
{
	#ifdef _DEBUG
		setDebugName( "ModalWidget");
	#endif
	setAlignment(alignUpperLeft, alignLowerRight, alignUpperLeft, alignLowerRight);

	// this element is a tab group
	setTabGroup(true);
}

bool ModalScreen::_canTakeFocus(Widget* target) const
{
    return (target && ((const Widget*)target == this // this element can take it
                        || isMyChild(target)    // own children also
                        || ( safety_cast< ModalScreen* >( target ) != 0 )// other modals also fine
                        || ( target->getParent() && ( safety_cast< ModalScreen* >( target->getParent() ) != 0) )))   // children of other modals will do
            ;
}

bool ModalScreen::isVisible() const
{
    // any parent invisible?
    Widget* parentElement = getParent();
    while ( parentElement )
    {
        if ( !parentElement->isVisible() )
            return false;

        parentElement = parentElement->getParent();
    }

    // if we have no children then the modal is probably abused as a way to block input
    if( getChildren().empty() )
    {
        return Widget::isVisible();
    }

    // any child visible?
    bool visible = false;
    ConstChildIterator it = getChildren().begin();
    for (; it != getChildren().end(); ++it)
    {
        if ( (*it)->isVisible() )
        {
            visible = true;
            break;
        }
    }
    return visible;
}

bool ModalScreen::isPointInside(const Point& point) const
{
    return true;
}

//! called if an event happened.
bool ModalScreen::onEvent(const NEvent& event)
{
  if (!isEnabled() || !isVisible() )
    return Widget::onEvent(event);

	switch(event.EventType)
	{
	case sEventGui:
		switch(event.GuiEvent.EventType)
		{
		case guiElementFocused:
			if ( !_canTakeFocus(event.GuiEvent.Caller))
			{
				if ( !getChildren().empty() )
					(*getChildren().begin())->setFocus();
				else
					setFocus();
			}
			Widget::onEvent(event);
			return false;

		case guiElementFocusLost:
			if ( !_canTakeFocus(event.GuiEvent.Element))
			{
				if ( isMyChild(event.GuiEvent.Caller) )
				{
					if ( !getChildren().empty() )
						(*getChildren().begin())->setFocus();
					else
						setFocus();
				}
				else
				{
					_mouseDownTime = DateTime::getElapsedTime();
				}
				return true;
			}
			else
			{
				return Widget::onEvent(event);
			}

		case guiElementClosed:
			// do not interfere with children being removed
			return Widget::onEvent(event);

		default:
		break;
		}
		break;
	case sEventMouse:
		if( event.MouseEvent.Event == mouseLbtnPressed )
		{
			_mouseDownTime = DateTime::getElapsedTime();
		}
	
	default:
	break;
	}

	Widget::onEvent(event);	// anyone knows why events are passed on here? Causes p.e. problems when this is child of a CGUIWindow.

	return true; // absorb everything else
}


//! draws the element and its children
void ModalScreen::draw( GfxEngine& painter )
{
	unsigned int now = DateTime::getElapsedTime();

	if( now - _mouseDownTime < 300 && (now / 70) % 2 )
	{
		Rect r;

		Widget::Widgets children = getChildren();
		foreach( Widget* w, children )
		{
			if( w->isVisible())
			{
				r = w->getAbsoluteRect();
				r.LowerRightCorner += Point( 1, 1 );
				r.UpperLeftCorner -= Point( 1, 1 );

				//painter.drawRectangle( 0xffc0c0c0, r, &getAbsoluteClippingRectRef() );
			}
		}
	}

	Widget::draw( painter );
}

//! Removes a child.
void ModalScreen::removeChild(Widget* child)
{
	Widget::removeChild(child);

	if (getChildren().empty())
	{
		deleteLater();
	}
}


//! adds a child
void ModalScreen::addChild(Widget* child)
{
	Widget::addChild(child);
	_environment->setFocus(child);
}


void ModalScreen::_resizeEvent()
{
	//setGeometry( RectF( 0, 0, 1, 1 ) );
}

}//end namespace gui
