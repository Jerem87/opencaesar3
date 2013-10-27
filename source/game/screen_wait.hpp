#ifndef SCREEN_WAIT_HPP
#define SCREEN_WAIT_HPP

#include "screen.hpp"
#include <memory>

class GfxEngine;
class GuiEnv;

// displays a background image
class ScreenWait: public Screen
{
public:
    ScreenWait();
    ~ScreenWait();

    void initialize();

    virtual void draw();

protected:
	int getResult() const;

private:
    class Impl;
    std::auto_ptr< Impl > _d;
};


#endif
