// CurrentWindowCmd.cc for Fluxbox - an X11 Window manager
// Copyright (c) 2003 - 2006 Henrik Kinnunen (fluxgen at fluxbox dot org)
//                and Simon Bowden (rathnor at users.sourceforge.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "CurrentWindowCmd.hh"

#include "fluxbox.hh"
#include "Layer.hh"
#include "Window.hh"
#include "WindowCmd.hh"
#include "Screen.hh"
#include "TextDialog.hh"
#include "WinClient.hh"

#include "FocusControl.hh"
#include "FbTk/CommandParser.hh"
#include "FbTk/I18n.hh"
#include "FbTk/stringstream.hh"
#include "FbTk/StringUtil.hh"

using FbTk::Command;

namespace {

FbTk::Command<void> *createCurrentWindowCmd(const std::string &command,
                                      const std::string &args, bool trusted) {
    if (command == "minimizewindow" || command == "minimize" || command == "iconify")
        return new CurrentWindowCmd(&FluxboxWindow::iconify);
    else if (command == "maximizewindow" || command == "maximize")
        return new CurrentWindowCmd(&FluxboxWindow::maximizeFull);
    else if (command == "maximizevertical")
        return new CurrentWindowCmd(&FluxboxWindow::maximizeVertical);
    else if (command == "maximizehorizontal")
        return new CurrentWindowCmd(&FluxboxWindow::maximizeHorizontal);
    else if (command == "raise")
        return new CurrentWindowCmd(&FluxboxWindow::raise);
    else if (command == "raiselayer")
        return new CurrentWindowCmd(&FluxboxWindow::raiseLayer);
    else if (command == "lower")
        return new CurrentWindowCmd(&FluxboxWindow::lower);
    else if (command == "lowerlayer")
        return new CurrentWindowCmd(&FluxboxWindow::lowerLayer);
    else if (command == "close")
        return new CurrentWindowCmd(&FluxboxWindow::close);
    else if (command == "killwindow" || command == "kill")
        return new CurrentWindowCmd(&FluxboxWindow::kill);
    else if (command == "shade" || command == "shadewindow")
        return new CurrentWindowCmd(&FluxboxWindow::shade);
    else if (command == "shadeon" )
        return new CurrentWindowCmd(&FluxboxWindow::shadeOn);
    else if (command == "shadeoff" )
        return new CurrentWindowCmd(&FluxboxWindow::shadeOff);
    else if (command == "stick" || command == "stickwindow")
        return new CurrentWindowCmd(&FluxboxWindow::stick);
    else if (command == "toggledecor")
        return new CurrentWindowCmd(&FluxboxWindow::toggleDecoration);
    else if (command == "nexttab")
        return new CurrentWindowCmd(&FluxboxWindow::nextClient);
    else if (command == "prevtab")
        return new CurrentWindowCmd(&FluxboxWindow::prevClient);
    else if (command == "movetableft")
        return new CurrentWindowCmd(&FluxboxWindow::moveClientLeft);
    else if (command == "movetabright")
        return new CurrentWindowCmd(&FluxboxWindow::moveClientRight);
    else if (command == "detachclient")
        return new CurrentWindowCmd(&FluxboxWindow::detachCurrentClient);
    else if (command == "windowmenu")
        return new CurrentWindowCmd(&FluxboxWindow::popupMenu);   
    return 0;
}

REGISTER_COMMAND_PARSER(minimizewindow, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(minimize, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(iconify, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(maximizewindow, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(maximize, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(maximizevertical, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(maximizehorizontal, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(raise, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(raiselayer, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(lower, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(lowerlayer, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(close, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(killwindow, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(kill, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(shade, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(shadewindow, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(shadeon, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(shadeoff, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(stick, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(stickwindow, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(toggledecor, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(nexttab, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(prevtab, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(movetableft, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(movetabright, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(detachclient, createCurrentWindowCmd, void);
REGISTER_COMMAND_PARSER(windowmenu, createCurrentWindowCmd, void);

}; // end anonymous namespace

void WindowHelperCmd::execute() {
    if (WindowCmd<void>::window() || FocusControl::focusedFbWindow())
        real_execute();
}

FluxboxWindow &WindowHelperCmd::fbwindow() {
    // will exist from execute above
    FluxboxWindow *tmp = WindowCmd<void>::window();
    if (tmp) return *tmp;
    return *FocusControl::focusedFbWindow();
}

bool WindowHelperBoolCmd::execute() {
    if (WindowCmd<void>::window() || FocusControl::focusedFbWindow())
        return real_execute();
    return false;
}

FluxboxWindow &WindowHelperBoolCmd::fbwindow() {
    // will exist from execute above
    FluxboxWindow *tmp = WindowCmd<void>::window();
    if (tmp) return *tmp;
    return *FocusControl::focusedFbWindow();
}

WinClient &WindowHelperBoolCmd::winclient() {
    // will exist from execute above
    WinClient *tmp = WindowCmd<void>::client();
    if (tmp) return *tmp;
    return *FocusControl::focusedWindow();
}

void CurrentWindowCmd::real_execute() {
    (fbwindow().*m_action)();
}

namespace {

FbTk::Command<void> *parseIntCmd(const string &command, const string &args,
                           bool trusted) {
    int num = 1;
    FbTk_istringstream iss(args.c_str());
    iss >> num;
    if (command == "sethead")
        return new SetHeadCmd(num);
    else if (command == "tab")
        return new GoToTabCmd(num);
    else if (command == "sendtonextworkspace")
        return new SendToNextWorkspaceCmd(num);
    else if (command == "sendtoprevworkspace")
        return new SendToNextWorkspaceCmd(-num);
    else if (command == "taketonextworkspace")
        return new SendToNextWorkspaceCmd(num, true);
    else if (command == "taketoprevworkspace")
        return new SendToNextWorkspaceCmd(-num, true);
    else if (command == "sendtoworkspace")
        return new SendToWorkspaceCmd(num);
    else if (command == "taketoworkspace")
        return new SendToWorkspaceCmd(num, true);
    else if (command == "sendtonexthead")
        return new SendToNextHeadCmd(num);
    else if (command == "sendtoprevhead")
        return new SendToNextHeadCmd(-num);
    return 0;
}

REGISTER_COMMAND_PARSER(sethead, parseIntCmd, void);
REGISTER_COMMAND_PARSER(tab, parseIntCmd, void);
REGISTER_COMMAND_PARSER(sendtonextworkspace, parseIntCmd, void);
REGISTER_COMMAND_PARSER(sendtoprevworkspace, parseIntCmd, void);
REGISTER_COMMAND_PARSER(taketonextworkspace, parseIntCmd, void);
REGISTER_COMMAND_PARSER(taketoprevworkspace, parseIntCmd, void);
REGISTER_COMMAND_PARSER(sendtoworkspace, parseIntCmd, void);
REGISTER_COMMAND_PARSER(taketoworkspace, parseIntCmd, void);
REGISTER_COMMAND_PARSER(sendtonexthead, parseIntCmd, void);
REGISTER_COMMAND_PARSER(sendtoprevhead, parseIntCmd, void);

FbTk::Command<void> *parseFocusCmd(const string &command, const string &args,
                                   bool trusted) {
    ClientPattern pat(args.c_str());
    if (!pat.error())
        return FbTk::CommandParser<void>::instance().parse("GoToWindow 1 " +
                                                           args);
    return new CurrentWindowCmd((CurrentWindowCmd::Action)
                                    &FluxboxWindow::focus);
}

REGISTER_COMMAND_PARSER(activate, parseFocusCmd, void);
REGISTER_COMMAND_PARSER(focus, parseFocusCmd, void);

}; // end anonymous namespace

void SetHeadCmd::real_execute() {
    int num = m_head;
    int total = fbwindow().screen().numHeads();
    if (num < 0) num += total + 1;
    if (num < 1) num = 1;
    if (num > total) num = total;
    fbwindow().setOnHead(num);
}

void SendToWorkspaceCmd::real_execute() {
    int num = m_workspace_num;
    int total = fbwindow().screen().numberOfWorkspaces();
    if (num < 0) num += total + 1;
    if (num < 1) num = 1;
    if (num > total) num = total;
    fbwindow().screen().sendToWorkspace(num-1, &fbwindow(), m_take);
}

void SendToNextWorkspaceCmd::real_execute() {
    int total = fbwindow().screen().numberOfWorkspaces();
    const int ws_nr = (total + (fbwindow().workspaceNumber() + m_delta % total)) % total;
    fbwindow().screen().sendToWorkspace(ws_nr, &fbwindow(), m_take);
}

void SendToNextHeadCmd::real_execute() {
    int total = fbwindow().screen().numHeads();
    if (total < 2)
        return;
    int num = (total + fbwindow().getOnHead() - 1 + (m_delta % total)) % total;
    fbwindow().setOnHead(1 + num);
}

void GoToTabCmd::real_execute() {
    int num = m_tab_num;
    if (num < 0) num += fbwindow().numClients() + 1;
    if (num < 1) num = 1;
    if (num > fbwindow().numClients()) num = fbwindow().numClients();

    FluxboxWindow::ClientList::iterator it = fbwindow().clientList().begin();

    while (--num > 0) ++it;

    (*it)->focus();
}

REGISTER_COMMAND(startmoving, StartMovingCmd, void);

void StartMovingCmd::real_execute() {
    const XEvent &last = Fluxbox::instance()->lastEvent();
    if (last.type == ButtonPress) {
        const XButtonEvent &be = last.xbutton;
        fbwindow().startMoving(be.x_root, be.y_root);
    }
}

FbTk::Command<void> *StartResizingCmd::parse(const string &cmd, const string &args,
                                       bool trusted) {
    FluxboxWindow::ResizeModel mode = FluxboxWindow::DEFAULTRESIZE;
    std::vector<string> tokens;
    FbTk::StringUtil::stringtok<std::vector<string> >(tokens, args);
    if (!tokens.empty()) {
        string arg = FbTk::StringUtil::toLower(tokens[0]);
        if (arg == "nearestcorner")
            mode = FluxboxWindow::QUADRANTRESIZE;
        else if (arg == "nearestedge")
            mode = FluxboxWindow::NEARESTEDGERESIZE;
        else if (arg == "center")
            mode = FluxboxWindow::CENTERRESIZE;
        else if (arg == "topleft")
            mode = FluxboxWindow::TOPLEFTRESIZE;
        else if (arg == "top")
            mode = FluxboxWindow::TOPRESIZE;
        else if (arg == "topright")
            mode = FluxboxWindow::TOPRIGHTRESIZE;
        else if (arg == "left")
            mode = FluxboxWindow::LEFTRESIZE;
        else if (arg == "right")
            mode = FluxboxWindow::RIGHTRESIZE;
        else if (arg == "bottomleft")
            mode = FluxboxWindow::BOTTOMLEFTRESIZE;
        else if (arg == "bottom")
            mode = FluxboxWindow::BOTTOMRESIZE;
        else if (arg == "bottomright")
            mode = FluxboxWindow::BOTTOMRIGHTRESIZE;
    }
    return new StartResizingCmd(mode);
}

REGISTER_COMMAND_PARSER(startresizing, StartResizingCmd::parse, void);

void StartResizingCmd::real_execute() {
    const XEvent &last = Fluxbox::instance()->lastEvent();
    if (last.type == ButtonPress) {
        const XButtonEvent &be = last.xbutton;
        int x = be.x_root - fbwindow().x()
                - fbwindow().frame().window().borderWidth();
        int y = be.y_root - fbwindow().y()
                - fbwindow().frame().window().borderWidth();
        fbwindow().startResizing(x, y, fbwindow().getResizeDirection(x, y, m_mode));
    }
}

REGISTER_COMMAND(starttabbing, StartTabbingCmd, void);

void StartTabbingCmd::real_execute() {
    const XEvent &last = Fluxbox::instance()->lastEvent();
    if (last.type == ButtonPress) {
        const XButtonEvent &be = last.xbutton;
        fbwindow().startTabbing(be);
    }
}

FbTk::Command<void> *MoveCmd::parse(const string &command, const string &args,
                              bool trusted) {
    FbTk_istringstream is(args.c_str());
    int dx = 0, dy = 0;
    is >> dx >> dy;

    if (command == "moveright")
        dy = 0;
    else if (command == "moveleft") {
        dy = 0;
        dx = -dx;
    } else if (command == "movedown") {
        dy = dx;
        dx = 0;
    } else if (command == "moveup") {
        dy = -dx;
        dx = 0;
    }
    return new MoveCmd(dx, dy);
}

REGISTER_COMMAND_PARSER(move, MoveCmd::parse, void);
REGISTER_COMMAND_PARSER(moveright, MoveCmd::parse, void);
REGISTER_COMMAND_PARSER(moveleft, MoveCmd::parse, void);
REGISTER_COMMAND_PARSER(moveup, MoveCmd::parse, void);
REGISTER_COMMAND_PARSER(movedown, MoveCmd::parse, void);

MoveCmd::MoveCmd(const int step_size_x, const int step_size_y) :
  m_step_size_x(step_size_x), m_step_size_y(step_size_y) { }

void MoveCmd::real_execute() {
  fbwindow().move(
      fbwindow().x() + m_step_size_x,
      fbwindow().y() + m_step_size_y);
}

FbTk::Command<void> *ResizeCmd::parse(const string &command, const string &args,
                                bool trusted) {
    FbTk_istringstream is(args.c_str());
    int dx = 0, dy = 0;
    is >> dx >> dy;
    if (command == "resizehorizontal")
        dy = 0;
    else if (command == "resizevertical") {
        dy = dx;
        dx = 0;
    }

    if (command == "resizeto")
        return new ResizeToCmd(dx, dy);
    return new ResizeCmd(dx, dy);
}

REGISTER_COMMAND_PARSER(resize, ResizeCmd::parse, void);
REGISTER_COMMAND_PARSER(resizeto, ResizeCmd::parse, void);
REGISTER_COMMAND_PARSER(resizehorizontal, ResizeCmd::parse, void);
REGISTER_COMMAND_PARSER(resizevertical, ResizeCmd::parse, void);

ResizeCmd::ResizeCmd(const int step_size_x, const int step_size_y) :
    m_step_size_x(step_size_x), m_step_size_y(step_size_y) { }

void ResizeCmd::real_execute() {

    int w = std::max<int>(static_cast<int>(fbwindow().width() +
                                      m_step_size_x * fbwindow().winClient().widthInc()),
                     fbwindow().frame().titlebarHeight() * 2 + 10);
    int h = std::max<int>(static_cast<int>(fbwindow().height() +
                                      m_step_size_y * fbwindow().winClient().heightInc()),
                     fbwindow().frame().titlebarHeight() + 10);
    fbwindow().resize(w, h);
}

FbTk::Command<void> *MoveToCmd::parse(const string &cmd, const string &args,
                                bool trusted) {
    typedef std::vector<string> StringTokens;
    StringTokens tokens;
    FbTk::StringUtil::stringtok<StringTokens>(tokens, args);

    if (tokens.size() < 2)
        return 0;

    FluxboxWindow::ReferenceCorner refc = FluxboxWindow::LEFTTOP;
    int x = 0, y = 0;
    bool ignore_x = false, ignore_y = false;

    if (tokens[0][0] == '*')
        ignore_x = true;
    else
        x = atoi(tokens[0].c_str());

    if (tokens[1][0] == '*' && !ignore_x)
        ignore_y = true;
    else
        y = atoi(tokens[1].c_str());

    if (tokens.size() >= 3) {
        refc = FluxboxWindow::getCorner(tokens[2]);
        if (refc == FluxboxWindow::ERROR)
            refc = FluxboxWindow::LEFTTOP;
    }

    return new MoveToCmd(x, y, ignore_x, ignore_y, refc);
}

REGISTER_COMMAND_PARSER(moveto, MoveToCmd::parse, void);

void MoveToCmd::real_execute() {
    int x = m_pos_x, y = m_pos_y;

    fbwindow().translateCoords(x, y, m_corner);
    if (m_ignore_x)
        x = fbwindow().x();
    if (m_ignore_y)
        y = fbwindow().y();

    fbwindow().move(x, y);
}


ResizeToCmd::ResizeToCmd(const int step_size_x, const int step_size_y) :
    m_step_size_x(step_size_x), m_step_size_y(step_size_y) { }

void ResizeToCmd::real_execute() {
    if (m_step_size_x > 0 && m_step_size_y > 0)
        fbwindow().resize(m_step_size_x, m_step_size_y);
}

REGISTER_COMMAND(fullscreen, FullscreenCmd, void);

void FullscreenCmd::real_execute() {
    fbwindow().setFullscreen(!fbwindow().isFullscreen());
}

FbTk::Command<void> *SetLayerCmd::parse(const string &command,
                                        const string &args, bool trusted) {
    int l = Layer::getNumFromString(args);
    return (l == -1) ? 0 : new SetLayerCmd(l);
}

REGISTER_COMMAND_PARSER(setlayer, SetLayerCmd::parse, void);

void SetLayerCmd::real_execute() {
    fbwindow().moveToLayer(m_layer);
}

namespace {
class SetTitleDialog: public TextDialog {
public:
    SetTitleDialog(FluxboxWindow &win, const string &title):
        TextDialog(win.screen(), title), window(win) {
        setText(win.title());
    }

private:
    void exec(const std::string &text) {
        window.winClient().setTitle(text);
    }

    FluxboxWindow &window;
};
} // end anonymous namespace

REGISTER_COMMAND(settitledialog, SetTitleDialogCmd, void);

void SetTitleDialogCmd::real_execute() {
    _FB_USES_NLS;

    SetTitleDialog *win = new SetTitleDialog(fbwindow(),
            _FB_XTEXT(Windowmenu, SetTitle, "Set Title",
                      "Change the title of the window"));
    win->show();
}

REGISTER_COMMAND_WITH_ARGS(settitle, SetTitleCmd, void);

void SetTitleCmd::real_execute() {
    fbwindow().winClient().setTitle(title);
}

REGISTER_COMMAND_WITH_ARGS(setdecor, SetDecorCmd, void);

SetDecorCmd::SetDecorCmd(const std::string &args):
    m_mask(WindowState::getDecoMaskFromString(args)) { }

void SetDecorCmd::real_execute() {
    fbwindow().setDecorationMask(m_mask);
}

FbTk::Command<void> *SetAlphaCmd::parse(const string &command, const string &args,
                                  bool trusted) {
    typedef std::vector<string> StringTokens;
    StringTokens tokens;
    FbTk::StringUtil::stringtok<StringTokens>(tokens, args);

    int focused, unfocused;
    bool relative, un_rel;

    if (tokens.empty()) { // set default alpha
        focused = unfocused = 256;
        relative = un_rel = false;
    } else {
        relative = un_rel = (tokens[0][0] == '+' || tokens[0][0] == '-');
        focused = unfocused = atoi(tokens[0].c_str());
    }

    if (tokens.size() > 1) { // set different unfocused alpha
        un_rel = (tokens[1][0] == '+' || tokens[1][0] == '-');
        unfocused = atoi(tokens[1].c_str());
    }

    return new SetAlphaCmd(focused, relative, unfocused, un_rel);
}

REGISTER_COMMAND_PARSER(setalpha, SetAlphaCmd::parse, void);

SetAlphaCmd::SetAlphaCmd(int focused, bool relative,
                         int unfocused, bool un_relative) :
    m_focus(focused), m_unfocus(unfocused),
    m_relative(relative), m_un_relative(un_relative) { }

void SetAlphaCmd::real_execute() {
    if (m_focus == 256 && m_unfocus == 256) {
        // made up signal to return to default
        fbwindow().setDefaultAlpha();
        return;
    }

    int new_alpha;
    if (m_relative) {
        new_alpha = fbwindow().getFocusedAlpha() + m_focus;
        if (new_alpha < 0) new_alpha = 0;
        if (new_alpha > 255) new_alpha = 255;
        fbwindow().setFocusedAlpha(new_alpha);
    } else
        fbwindow().setFocusedAlpha(m_focus);

    if (m_un_relative) {
        new_alpha = fbwindow().getUnfocusedAlpha() + m_unfocus;
        if (new_alpha < 0) new_alpha = 0;
        if (new_alpha > 255) new_alpha = 255;
        fbwindow().setUnfocusedAlpha(new_alpha);
    } else
        fbwindow().setUnfocusedAlpha(m_unfocus);
}

REGISTER_COMMAND_WITH_ARGS(matches, MatchCmd, bool);

bool MatchCmd::real_execute() {
    return m_pat.match(winclient());
}
