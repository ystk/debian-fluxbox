// FbCommands.cc for Fluxbox
// Copyright (c) 2003 - 2006 Henrik Kinnunen (fluxgen at fluxbox dot org)
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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include "FbCommands.hh"
#include "fluxbox.hh"
#include "Screen.hh"
#include "CommandDialog.hh"
#include "FocusControl.hh"
#include "Workspace.hh"
#include "Window.hh"
#include "Keys.hh"
#include "MenuCreator.hh"

#include "FbTk/Theme.hh"
#include "FbTk/Menu.hh"
#include "FbTk/CommandParser.hh"
#include "FbTk/StringUtil.hh"
#include "FbTk/stringstream.hh"

#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <set>

#ifdef HAVE_CSTDLIB
  #include <cstdlib>
#else
  #include <stdlib.h>
#endif
#ifdef HAVE_CSTRING
  #include <cstring>
#else
  #include <string.h>
#endif


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#if defined(__EMX__) && defined(HAVE_PROCESS_H)
#include <process.h> // for P_NOWAIT
#endif // __EMX__

using std::string;
using std::pair;
using std::set;
using std::ofstream;
using std::endl;
using std::ios;

namespace {

void showMenu(const BScreen &screen, FbTk::Menu &menu) {

    // check if menu has changed
    if (typeid(menu) == typeid(FbMenu)) {
        FbMenu *fbmenu = static_cast<FbMenu *>(&menu);
        if (fbmenu->reloadHelper())
            fbmenu->reloadHelper()->checkReload();
    }

    FbMenu::setWindow(FocusControl::focusedFbWindow());

    Window root_ret; // not used
    Window window_ret; // not used

    int rx = 0, ry = 0;
    int wx, wy; // not used
    unsigned int mask; // not used

    XQueryPointer(menu.fbwindow().display(),
                  screen.rootWindow().window(), &root_ret, &window_ret,
                  &rx, &ry, &wx, &wy, &mask);

    int borderw = menu.fbwindow().borderWidth();
    int head = screen.getHead(rx, ry);

    menu.updateMenu();
    pair<int, int> m =
        screen.clampToHead(head,
                           rx - menu.width() / 2,
                           ry - menu.titleWindow().height() / 2,
                           menu.width() + 2*borderw,
                           menu.height() + 2*borderw);

    menu.move(m.first, m.second);
    menu.setScreen(screen.getHeadX(head),
                   screen.getHeadY(head),
                   screen.getHeadWidth(head),
                   screen.getHeadHeight(head));

    menu.show();
    menu.grabInputFocus();
}

}

namespace FbCommands {

using FbTk::Command;

REGISTER_UNTRUSTED_COMMAND_WITH_ARGS(exec, FbCommands::ExecuteCmd, void);
REGISTER_UNTRUSTED_COMMAND_WITH_ARGS(execute, FbCommands::ExecuteCmd, void);
REGISTER_UNTRUSTED_COMMAND_WITH_ARGS(execcommand, FbCommands::ExecuteCmd, void);

ExecuteCmd::ExecuteCmd(const string &cmd, int screen_num):m_cmd(cmd), m_screen_num(screen_num) {

}

void ExecuteCmd::execute() {
#ifndef __EMX__
    run();
#else //   __EMX__
    spawnlp(P_NOWAIT, "cmd.exe", "cmd.exe", "/c", m_cmd.c_str(), static_cast<void*>(NULL));
#endif // !__EMX__

}

int ExecuteCmd::run() {
    pid_t pid = fork();
    if (pid)
        return pid;

    string displaystring("DISPLAY=");
    displaystring += DisplayString(FbTk::App::instance()->display());
    char intbuff[64];
    int screen_num = m_screen_num;
    if (screen_num < 0) {
        if (Fluxbox::instance()->mouseScreen() == 0)
            screen_num = 0;
        else
            screen_num = Fluxbox::instance()->mouseScreen()->screenNumber();
    }

    sprintf(intbuff, "%d", screen_num);

    // get shell path from the environment
    // this process exits immediately, so we don't have to worry about memleaks
    const char *shell = getenv("SHELL");
    if (!shell)
        shell = "/bin/sh";

    // remove last number of display and add screen num
    displaystring.erase(displaystring.size()-1);
    displaystring += intbuff;

    setsid();
    putenv(const_cast<char *>(displaystring.c_str()));
    execl(shell, shell, "-c", m_cmd.c_str(), static_cast<void*>(NULL));
    exit(EXIT_SUCCESS);

    return pid; // compiler happy -> we are happy ;)
}

FbTk::Command<void> *ExportCmd::parse(const string &command, const string &args,
                                bool trusted) {
    string name = args;
    FbTk::StringUtil::removeFirstWhitespace(name);
    if (command != "setresourcevalue")
        FbTk::StringUtil::removeTrailingWhitespace(name);
    size_t pos = name.find_first_of(command == "export" ? "=" : " \t");
    if (pos == string::npos || pos == name.size() || !trusted)
        return 0;

    string value = name.substr(pos + 1);
    name = name.substr(0, pos);
    if (command == "setresourcevalue")
        return new SetResourceValueCmd(name, value);
    return new ExportCmd(name, value);
}

REGISTER_COMMAND_PARSER(setenv, ExportCmd::parse, void);
REGISTER_COMMAND_PARSER(export, ExportCmd::parse, void);
REGISTER_COMMAND_PARSER(setresourcevalue, ExportCmd::parse, void);

ExportCmd::ExportCmd(const string& name, const string& value) :
    m_name(name), m_value(value) {
}

void ExportCmd::execute() {

    // the setenv()-routine is not everywhere available and
    // putenv() doesnt manage the strings in the environment
    // and hence we have to do that on our own to avoid memleaking
    static set<char*> stored;
    char* newenv = new char[m_name.size() + m_value.size() + 2];
    if (newenv) {

        char* oldenv = getenv(m_name.c_str());

        // oldenv points to the value .. we have to go back a bit
        if (oldenv && stored.find(oldenv - (m_name.size() + 1)) != stored.end())
            oldenv -= (m_name.size() + 1);
        else
            oldenv = NULL;

        memset(newenv, 0, m_name.size() + m_value.size() + 2);
        strcat(newenv, m_name.c_str());
        strcat(newenv, "=");
        strcat(newenv, m_value.c_str());

        if (putenv(newenv) == 0) {
            if (oldenv) {
                stored.erase(oldenv);
                delete[] oldenv;
            }
            stored.insert(newenv);
        }
    }
}

REGISTER_COMMAND(exit, FbCommands::ExitFluxboxCmd, void);
REGISTER_COMMAND(quit, FbCommands::ExitFluxboxCmd, void);

void ExitFluxboxCmd::execute() {
    Fluxbox::instance()->shutdown();
}

REGISTER_COMMAND(saverc, FbCommands::SaveResources, void);

void SaveResources::execute() {
    Fluxbox::instance()->save_rc();
}

REGISTER_COMMAND_PARSER(restart, RestartFluxboxCmd::parse, void);

FbTk::Command<void> *RestartFluxboxCmd::parse(const string &command,
        const string &args, bool trusted) {
    if (!trusted && !args.empty())
        return 0;
    return new RestartFluxboxCmd(args);
}

RestartFluxboxCmd::RestartFluxboxCmd(const string &cmd):m_cmd(cmd){
}

void RestartFluxboxCmd::execute() {
    if (m_cmd.empty())
        Fluxbox::instance()->restart();
    else
        Fluxbox::instance()->restart(m_cmd.c_str());
}

REGISTER_COMMAND(reconfigure, FbCommands::ReconfigureFluxboxCmd, void);
REGISTER_COMMAND(reconfig, FbCommands::ReconfigureFluxboxCmd, void);

void ReconfigureFluxboxCmd::execute() {
    Fluxbox::instance()->reconfigure();
}

REGISTER_COMMAND(reloadstyle, FbCommands::ReloadStyleCmd, void);

void ReloadStyleCmd::execute() {
    SetStyleCmd cmd(Fluxbox::instance()->getStyleFilename());
    cmd.execute();
}

REGISTER_COMMAND_WITH_ARGS(setstyle, FbCommands::SetStyleCmd, void);

SetStyleCmd::SetStyleCmd(const string &filename):m_filename(filename) {

}

void SetStyleCmd::execute() {
    if (FbTk::ThemeManager::instance().load(m_filename,
        Fluxbox::instance()->getStyleOverlayFilename())) {
        Fluxbox::instance()->saveStyleFilename(m_filename.c_str());
        Fluxbox::instance()->save_rc();
    }
}

REGISTER_COMMAND_WITH_ARGS(keymode, FbCommands::KeyModeCmd, void);

KeyModeCmd::KeyModeCmd(const string &arguments):m_keymode(arguments),m_end_args("None Escape") {
    string::size_type second_pos = m_keymode.find_first_of(" \t", 0);
    if (second_pos != string::npos) {
        // ok we have arguments, parsing them here
        m_end_args = m_keymode.substr(second_pos);
        m_keymode.erase(second_pos); // remove argument from command
    }
    if (m_keymode != "default")
        Fluxbox::instance()->keys()->addBinding(m_keymode + ": " + m_end_args + " :keymode default");
}

void KeyModeCmd::execute() {
    Fluxbox::instance()->keys()->keyMode(m_keymode);
}

REGISTER_COMMAND(hidemenus, FbCommands::HideMenuCmd, void);

void HideMenuCmd::execute() {
    FbTk::Menu::hideShownMenu();
}

FbTk::Command<void> *ShowClientMenuCmd::parse(const string &command,
                                        const string &args, bool trusted) {
    int opts;
    string pat;
    FocusableList::parseArgs(args, opts, pat);
    return new ShowClientMenuCmd(opts, pat);
}

REGISTER_COMMAND_PARSER(clientmenu, ShowClientMenuCmd::parse, void);

void ShowClientMenuCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;

    // TODO: ClientMenu only accepts lists of FluxboxWindows for now
    //       when that's fixed, use a FocusableList for m_list
    const FocusableList *list =
        FocusableList::getListFromOptions(*screen, m_option);
    m_list.clear();
    FocusControl::Focusables::const_iterator it = list->clientList().begin(),
                                             it_end = list->clientList().end();
    for (; it != it_end; ++it) {
        if (typeid(**it) == typeid(FluxboxWindow) && m_pat.match(**it))
            m_list.push_back(static_cast<FluxboxWindow *>(*it));
    }

    m_menu.reset(new ClientMenu(*screen, m_list, 0));
    ::showMenu(*screen, *m_menu.get());
}

REGISTER_COMMAND_WITH_ARGS(custommenu, FbCommands::ShowCustomMenuCmd, void);

ShowCustomMenuCmd::ShowCustomMenuCmd(const string &arguments) : custom_menu_file(arguments) {}

void ShowCustomMenuCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;

    if (!m_menu.get() || screen->screenNumber() != m_menu->screenNumber()) {
        m_menu.reset(screen->createMenu(""));
        m_menu->setReloadHelper(new FbTk::AutoReloadHelper());
        m_menu->reloadHelper()->setReloadCmd(FbTk::RefCount<FbTk::Command<void> >(new FbTk::SimpleCommand<ShowCustomMenuCmd>(*this, &ShowCustomMenuCmd::reload)));
        m_menu->reloadHelper()->setMainFile(custom_menu_file);
    } else
        m_menu->reloadHelper()->checkReload();

    ::showMenu(*screen, *m_menu.get());
}

void ShowCustomMenuCmd::reload() {
    m_menu->removeAll();
    m_menu->setLabel("");
    MenuCreator::createFromFile(custom_menu_file, *m_menu.get(), m_menu->reloadHelper());
}

REGISTER_COMMAND(rootmenu, FbCommands::ShowRootMenuCmd, void);

void ShowRootMenuCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;

    ::showMenu(*screen, screen->rootMenu());
}

REGISTER_COMMAND(workspacemenu, FbCommands::ShowWorkspaceMenuCmd, void);

void ShowWorkspaceMenuCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;

    ::showMenu(*screen, screen->workspaceMenu());
}

REGISTER_COMMAND_WITH_ARGS(setworkspacename, FbCommands::SetWorkspaceNameCmd, void);

SetWorkspaceNameCmd::SetWorkspaceNameCmd(const string &name, int spaceid):
    m_name(name), m_workspace(spaceid) {
    if (name.empty())
        m_name = "empty";
}

void SetWorkspaceNameCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0) {
        screen = Fluxbox::instance()->keyScreen();
        if (screen == 0)
            return;
    }

    if (m_workspace < 0) {
        screen->currentWorkspace()->setName(m_name);
    } else {
        Workspace *space = screen->getWorkspace(m_workspace);
        if (space == 0)
            return;
        space->setName(m_name);
    }
}

REGISTER_COMMAND(setworkspacenamedialog, FbCommands::WorkspaceNameDialogCmd, void);

void WorkspaceNameDialogCmd::execute() {

    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;

    CommandDialog *win = new CommandDialog(*screen, "Set Workspace Name:", "SetWorkspaceName ");
    win->setText(screen->currentWorkspace()->name());
    win->show();
}

REGISTER_COMMAND(commanddialog, FbCommands::CommandDialogCmd, void);

void CommandDialogCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;

    FbTk::FbWindow *win = new CommandDialog(*screen, "Fluxbox Command");
    win->show();
}


SetResourceValueCmd::SetResourceValueCmd(const string &resname,
                                         const string &value):
    m_resname(resname),
    m_value(value) {

}

void SetResourceValueCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;
    screen->resourceManager().setResourceValue(m_resname, m_value);
    Fluxbox::instance()->save_rc();
}

REGISTER_COMMAND(setresourcevaluedialog, FbCommands::SetResourceValueDialogCmd, void);

void SetResourceValueDialogCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;

    FbTk::FbWindow *win = new CommandDialog(*screen,  "Type resource name and the value", "SetResourceValue ");
    win->show();
};

REGISTER_UNTRUSTED_COMMAND_WITH_ARGS(bindkey, FbCommands::BindKeyCmd, void);

BindKeyCmd::BindKeyCmd(const string &keybind):m_keybind(keybind) { }

void BindKeyCmd::execute() {
    if (Fluxbox::instance()->keys() != 0) {
        if (Fluxbox::instance()->keys()->addBinding(m_keybind)) {
            ofstream ofile(Fluxbox::instance()->keys()->filename().c_str(), ios::app);
            if (!ofile)
                return;
            ofile<<m_keybind<<endl;
        }
    }
}

FbTk::Command<void> *DeiconifyCmd::parse(const string &command, const string &args,
                                   bool trusted) {
    FbTk_istringstream iss(args.c_str());
    string mode;
    string d;
    Destination dest;

    iss >> mode;
    if (iss.fail())
        mode="lastworkspace";
    mode= FbTk::StringUtil::toLower(mode);

    iss >> d;
    if (iss.fail())
        d="current";
    d = FbTk::StringUtil::toLower(d);
    if (d == "origin" )
        dest = ORIGIN;
    else if (d == "originquiet")
        dest = ORIGINQUIET;
    else
        dest = CURRENT;

    if (mode == "all")
        return new DeiconifyCmd(DeiconifyCmd::ALL, dest);
    else if (mode == "allworkspace")
        return new DeiconifyCmd(DeiconifyCmd::ALLWORKSPACE, dest);
    else if (mode == "last")
        return new DeiconifyCmd(DeiconifyCmd::LAST, dest);
    // lastworkspace, default
    return new DeiconifyCmd(DeiconifyCmd::LASTWORKSPACE, dest);
}

REGISTER_COMMAND_PARSER(deiconify, DeiconifyCmd::parse, void);

DeiconifyCmd::DeiconifyCmd(Mode mode,
                           Destination dest) : m_mode(mode), m_dest(dest) { }

void DeiconifyCmd::execute() {
    BScreen *screen = Fluxbox::instance()->mouseScreen();
    if (screen == 0)
        return;

    // we need to make a copy of the list of icons, or else our iterator can
    // become invalid
    BScreen::Icons icon_list = screen->iconList();
    BScreen::Icons::reverse_iterator it = icon_list.rbegin();
    BScreen::Icons::reverse_iterator itend= icon_list.rend();
    unsigned int workspace_num= screen->currentWorkspaceID();
    unsigned int old_workspace_num;

    const bool change_ws= m_dest == ORIGIN;

    switch(m_mode) {

    case ALL:
    case ALLWORKSPACE:
        for(; it != itend; it++) {
            old_workspace_num= (*it)->workspaceNumber();
            if (m_mode == ALL || old_workspace_num == workspace_num ||
                (*it)->isStuck()) {
                if (m_dest == ORIGIN || m_dest == ORIGINQUIET)
                    screen->sendToWorkspace(old_workspace_num, (*it), change_ws);
                (*it)->deiconify();
            }
        }
        break;

    case LAST:
    case LASTWORKSPACE:
    default:
        for (; it != itend; it++) {
            old_workspace_num= (*it)->workspaceNumber();
            if(m_mode == LAST || old_workspace_num == workspace_num ||
               (*it)->isStuck()) {
                if ((m_dest == ORIGIN || m_dest == ORIGINQUIET) &&
                    m_mode != LASTWORKSPACE)
                    screen->sendToWorkspace(old_workspace_num, (*it), change_ws);
                else
                    (*it)->deiconify();
                break;
            }
        }
        break;
    };
}

} // end namespace FbCommands
