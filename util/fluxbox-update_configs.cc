// fluxbox-update_configs.cc
// Copyright (c) 2007 Fluxbox Team (fluxgen at fluxbox dot org)
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

#include "../src/FbTk/I18n.hh"
#include "../src/FbTk/Resource.hh"
#include "../src/FbTk/StringUtil.hh"

#include "../src/defaults.hh"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif // HAVE_SIGNAL_H

//use GNU extensions
#ifndef         _GNU_SOURCE
#define         _GNU_SOURCE
#endif // _GNU_SOURCE

#ifdef HAVE_CSTRING
  #include <cstring>
#else
  #include <string.h>
#endif

#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <cstdlib>
#include <list>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::ifstream;
using std::ofstream;
using std::set;
using std::map;
using std::list;
using std::exit;
using std::getenv;

string read_file(string filename);
void write_file(string filename, string &contents);
void save_all_files();

int run_updates(int old_version, FbTk::ResourceManager &rm) {
    int new_version = old_version;

    FbTk::Resource<string> rc_keyfile(rm, "~/.fluxbox/keys",
            "session.keyFile", "Session.KeyFile");
    FbTk::Resource<string> rc_appsfile(rm, "~/.fluxbox/apps",
            "session.appsFile", "Session.AppsFile");

    string appsfilename = FbTk::StringUtil::expandFilename(*rc_appsfile);
    string keyfilename = FbTk::StringUtil::expandFilename(*rc_keyfile);

    if (old_version < 1) { // add mouse events to keys file

        string whole_keyfile = read_file(keyfilename);
        string new_keyfile = "";
        // let's put our new keybindings first, so they're easy to find
        new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
        new_keyfile += "OnDesktop Mouse1 :HideMenus\n";
        new_keyfile += "OnDesktop Mouse2 :WorkspaceMenu\n";
        new_keyfile += "OnDesktop Mouse3 :RootMenu\n";

        // scrolling on desktop needs to match user's desktop wheeling settings
        // hmmm, what are the odds that somebody wants this to be different on
        // different screens? the ability is going away until we make per-screen
        // keys files, anyway, so let's just use the first screen's setting
        FbTk::Resource<bool> rc_wheeling(rm, true,
                                         "session.screen0.desktopwheeling",
                                         "Session.Screen0.DesktopWheeling");
        FbTk::Resource<bool> rc_reverse(rm, false,
                                        "session.screen0.reversewheeling",
                                        "Session.Screen0.ReverseWheeling");
        if (*rc_wheeling) {
            if (*rc_reverse) { // if you ask me, this should have been default
                new_keyfile += "OnDesktop Mouse4 :PrevWorkspace\n";
                new_keyfile += "OnDesktop Mouse5 :NextWorkspace\n";
            } else {
                new_keyfile += "OnDesktop Mouse4 :NextWorkspace\n";
                new_keyfile += "OnDesktop Mouse5 :PrevWorkspace\n";
            }
        }
        new_keyfile += "\n"; // just for good looks
        new_keyfile += whole_keyfile; // don't forget user's old keybindings

        write_file(keyfilename, new_keyfile);
        new_version = 1;
    }

    if (old_version < 2) { // move groups entries to apps file
        FbTk::Resource<string> rc_groupfile(rm, "~/.fluxbox/groups",
                "session.groupFile", "Session.GroupFile");
        string groupfilename = FbTk::StringUtil::expandFilename(*rc_groupfile);
        string whole_groupfile = read_file(groupfilename);
        string whole_appsfile = read_file(appsfilename);
        string new_appsfile = "";

        list<string> lines;
        FbTk::StringUtil::stringtok(lines, whole_groupfile, "\n");

        list<string>::iterator line_it = lines.begin();
        list<string>::iterator line_it_end = lines.end();
        for (; line_it != line_it_end; ++line_it) {
            new_appsfile += "[group] (workspace=[current])\n";

            list<string> apps;
            FbTk::StringUtil::stringtok(apps, *line_it);

            list<string>::iterator it = apps.begin();
            list<string>::iterator it_end = apps.end();
            for (; it != it_end; ++it) {
                new_appsfile += " [app] (name=";
                new_appsfile += *it;
                new_appsfile += ")\n";
            }

            new_appsfile += "[end]\n";
        }

        new_appsfile += whole_appsfile;
        write_file(appsfilename, new_appsfile);
        new_version = 2;
    }

    if (old_version < 3) { // move toolbar wheeling to keys file
        string whole_keyfile = read_file(keyfilename);
        string new_keyfile = "";
        // let's put our new keybindings first, so they're easy to find
        new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
        bool keep_changes = false;

        // scrolling on toolbar needs to match user's toolbar wheeling settings
        FbTk::Resource<string> rc_wheeling(rm, "Off",
                                           "session.screen0.iconbar.wheelMode",
                                           "Session.Screen0.Iconbar.WheelMode");
        FbTk::Resource<bool> rc_screen(rm, true,
                                       "session.screen0.desktopwheeling",
                                       "Session.Screen0.DesktopWheeling");
        FbTk::Resource<bool> rc_reverse(rm, false,
                                        "session.screen0.reversewheeling",
                                        "Session.Screen0.ReverseWheeling");
        if (strcasecmp((*rc_wheeling).c_str(), "On") == 0 ||
            (strcasecmp((*rc_wheeling).c_str(), "Screen") == 0 && *rc_screen)) {
            keep_changes = true;
            if (*rc_reverse) { // if you ask me, this should have been default
                new_keyfile += "OnToolbar Mouse4 :PrevWorkspace\n";
                new_keyfile += "OnToolbar Mouse5 :NextWorkspace\n";
            } else {
                new_keyfile += "OnToolbar Mouse4 :NextWorkspace\n";
                new_keyfile += "OnToolbar Mouse5 :PrevWorkspace\n";
            }
        }
        new_keyfile += "\n"; // just for good looks
        new_keyfile += whole_keyfile; // don't forget user's old keybindings

        if (keep_changes)
            write_file(keyfilename, new_keyfile);
        new_version = 3;
    }

    if (old_version < 4) { // move modkey to keys file
        string whole_keyfile = read_file(keyfilename);
        string new_keyfile = "";
        // let's put our new keybindings first, so they're easy to find
        new_keyfile += "!mouse actions added by fluxbox-update_configs\n";

        // need to match user's resize model
        FbTk::Resource<string> rc_mode(rm, "Bottom",
                                           "session.screen0.resizeMode",
                                           "Session.Screen0.ResizeMode");
        FbTk::Resource<string> rc_modkey(rm, "Mod1",
                                       "session.modKey",
                                       "Session.ModKey");

        new_keyfile += "OnWindow " + *rc_modkey +
                       " Mouse1 :MacroCmd {Raise} {Focus} {StartMoving}\n";
        new_keyfile += "OnWindow " + *rc_modkey +
                       " Mouse3 :MacroCmd {Raise} {Focus} {StartResizing ";
        if (strcasecmp((*rc_mode).c_str(), "Quadrant") == 0) {
            new_keyfile += "NearestCorner}\n";
        } else if (strcasecmp((*rc_mode).c_str(), "Center") == 0) {
            new_keyfile += "Center}\n";
        } else {
            new_keyfile += "BottomRight}\n";
        }
        new_keyfile += "\n"; // just for good looks
        new_keyfile += whole_keyfile; // don't forget user's old keybindings

        write_file(keyfilename, new_keyfile);
        new_version = 4;
    }

    if (old_version < 5) { // window patterns for iconbar
        // this needs to survive after going out of scope
        // it won't get freed, but that's ok
        FbTk::Resource<string> *rc_mode =
            new FbTk::Resource<string>(rm, "Workspace",
                                       "session.screen0.iconbar.mode",
                                       "Session.Screen0.Iconbar.Mode");
        if (strcasecmp((**rc_mode).c_str(), "None") == 0)
            *rc_mode = "none";
        else if (strcasecmp((**rc_mode).c_str(), "Icons") == 0)
            *rc_mode = "{static groups} (minimized=yes)";
        else if (strcasecmp((**rc_mode).c_str(), "NoIcons") == 0)
            *rc_mode = "{static groups} (minimized=no)";
        else if (strcasecmp((**rc_mode).c_str(), "WorkspaceIcons") == 0)
            *rc_mode = "{static groups} (minimized=yes) (workspace)";
        else if (strcasecmp((**rc_mode).c_str(), "WorkspaceNoIcons") == 0)
            *rc_mode = "{static groups} (minimized=no) (workspace)";
        else if (strcasecmp((**rc_mode).c_str(), "AllWindows") == 0)
            *rc_mode = "{static groups}";
        else
            *rc_mode = "{static groups} (workspace)";

        new_version = 5;
    }

    if (old_version < 6) { // move titlebar actions to keys file
        string whole_keyfile = read_file(keyfilename);
        string new_keyfile = "";
        // let's put our new keybindings first, so they're easy to find
        new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
        new_keyfile += "OnTitlebar Double Mouse1 :Shade\n";
        new_keyfile += "OnTitlebar Mouse3 :WindowMenu\n";

        FbTk::Resource<bool> rc_reverse(rm, false,"session.screen0.reversewheeling", "Session.Screen0.ReverseWheeling");
        FbTk::Resource<std::string>  scroll_action(rm, "", "session.screen0.windowScrollAction", "Session.Screen0.WindowScrollAction");
        if (strcasecmp((*scroll_action).c_str(), "shade") == 0) {
            if (*rc_reverse) {
                new_keyfile += "OnTitlebar Mouse5 :ShadeOn\n";
                new_keyfile += "OnTitlebar Mouse4 :ShadeOff\n";
            } else {
                new_keyfile += "OnTitlebar Mouse4 :ShadeOn\n";
                new_keyfile += "OnTitlebar Mouse5 :ShadeOff\n";
            }
        } else if (strcasecmp((*scroll_action).c_str(), "nexttab") == 0) {
            if (*rc_reverse) {
                new_keyfile += "OnTitlebar Mouse5 :PrevTab\n";
                new_keyfile += "OnTitlebar Mouse4 :NextTab\n";
            } else {
                new_keyfile += "OnTitlebar Mouse4 :PrevTab\n";
                new_keyfile += "OnTitlebar Mouse5 :NextTab\n";
            }
        }

        new_keyfile += "\n"; // just for good looks
        new_keyfile += whole_keyfile; // don't forget user's old keybindings

        write_file(keyfilename, new_keyfile);

        new_version = 6;
    }

    if (old_version < 7) { // added StartTabbing command
        string whole_keyfile = read_file(keyfilename);
        string new_keyfile = "";
        // let's put our new keybindings first, so they're easy to find
        new_keyfile += "!mouse actions added by fluxbox-update_configs\n";
        new_keyfile += "OnTitlebar Mouse2 :StartTabbing\n\n";
        new_keyfile += whole_keyfile; // don't forget user's old keybindings

        write_file(keyfilename, new_keyfile);

        new_version = 7;
    }

    if (old_version < 8) { // disable icons in tabs for backwards compatibility
        FbTk::Resource<bool> *show =
            new FbTk::Resource<bool>(rm, false,
                                     "session.screen0.tabs.usePixmap",
                                     "Session.Screen0.Tabs.UsePixmap");
        if (!*show) // only change if the setting didn't already exist
            *show = false;
        new_version = 8;
    }

    if (old_version < 9) { // change format of slit placement menu
        FbTk::Resource<string> *placement =
            new FbTk::Resource<string>(rm, "BottomRight",
                                       "session.screen0.slit.placement",
                                       "Session.Screen0.Slit.Placement");
        FbTk::Resource<string> *direction =
            new FbTk::Resource<string>(rm, "Vertical",
                                       "session.screen0.slit.direction",
                                       "Session.Screen0.Slit.Direction");
        if (strcasecmp((**direction).c_str(), "vertical") == 0) {
            if (strcasecmp((**placement).c_str(), "BottomRight") == 0)
                *placement = "RightBottom";
            else if (strcasecmp((**placement).c_str(), "BottomLeft") == 0)
                *placement = "LeftBottom";
            else if (strcasecmp((**placement).c_str(), "TopRight") == 0)
                *placement = "RightTop";
            else if (strcasecmp((**placement).c_str(), "TopLeft") == 0)
                *placement = "LeftTop";
        }
        new_version = 9;
    }

    if (old_version < 10) { // update keys file for NextWindow syntax changes
        string whole_keyfile = read_file(keyfilename);

        size_t pos = 0;
        while (true) {
            const char *keyfile = whole_keyfile.c_str();
            const char *loc = 0;
            size_t old_pos = pos;
            // find the first position that matches any of next/prevwindow/group
            if ((loc = FbTk::StringUtil::strcasestr(keyfile + old_pos,
                                                    "nextwindow")))
                pos = (loc - keyfile) + 10;
            if ((loc = FbTk::StringUtil::strcasestr(keyfile + old_pos,
                                                    "prevwindow")))
                pos = (pos > old_pos && keyfile + pos < loc) ?
                       pos : (loc - keyfile) + 10;
            if ((loc = FbTk::StringUtil::strcasestr(keyfile + old_pos,
                                                    "nextgroup")))
                pos = (pos > old_pos && keyfile + pos < loc) ?
                       pos : (loc - keyfile) + 9;
            if ((loc = FbTk::StringUtil::strcasestr(keyfile + old_pos,
                                                    "prevgroup")))
                pos = (pos > old_pos && keyfile + pos < loc) ?
                       pos : (loc - keyfile) + 9;
            if (pos == old_pos)
                break;

            pos = whole_keyfile.find_first_not_of(" \t", pos);
            if (pos != std::string::npos && isdigit(whole_keyfile[pos])) {
                char *endptr = 0;
                unsigned int mask = strtoul(keyfile + pos, &endptr, 0);
                string insert = "";
                if ((mask & 9) == 9)
                    insert = "{static groups}";
                else if (mask & 1)
                    insert = "{groups}";
                else if (mask & 8)
                    insert = "{static}";
                if (mask & 2)
                    insert += " (stuck=no)";
                if (mask & 4)
                    insert += " (shaded=no)";
                if (mask & 16)
                    insert += " (minimized=no)";
                if (mask)
                    whole_keyfile.replace(pos, endptr - keyfile - pos, insert);
            }
        }

        write_file(keyfilename, whole_keyfile);
        new_version = 10;
    }

    return new_version;
}

int main(int argc, char **argv) {
    string rc_filename;
    set<string> style_filenames;
    int i = 1;
    pid_t fb_pid = 0;

    FbTk::NLSInit("fluxbox.cat");
    _FB_USES_NLS;

    for (; i < argc; i++) {
        if (strcmp(argv[i], "-rc") == 0) {
            // look for alternative rc file to use

            if ((++i) >= argc) {
                cerr<<_FB_CONSOLETEXT(main, RCRequiresArg,
                              "error: '-rc' requires an argument", "the -rc option requires a file argument")<<endl;
                exit(1);
            }

            rc_filename = argv[i];
        } else if (strcmp(argv[i], "-pid") == 0) {
            if ((++i) >= argc) {
                // need translations for this, too
                cerr<<"the -pid option requires a numeric argument"<<endl;
            } else
                fb_pid = atoi(argv[i]);
        } else if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "-h") == 0) {
            // no NLS translations yet -- we'll just have to use English for now
            cout << "  -rc <string>\t\t\tuse alternate resource file.\n"
                 << "  -pid <int>\t\t\ttell fluxbox to reload configuration.\n"
                 << "  -help\t\t\t\tdisplay this help text and exit.\n\n"
                 << endl;
            exit(0);
        }
    }

    if (rc_filename.empty())
        rc_filename = getenv("HOME") + string("/.fluxbox/init");

    FbTk::ResourceManager resource_manager(rc_filename.c_str(),false);
    if (!resource_manager.load(rc_filename.c_str())) {
        // couldn't load rc file
        cerr<<_FB_CONSOLETEXT(Fluxbox, CantLoadRCFile, "Failed to load database", "Failed trying to read rc file")<<":"<<rc_filename<<endl;
        cerr<<_FB_CONSOLETEXT(Fluxbox, CantLoadRCFileTrying, "Retrying with", "Retrying rc file loading with (the following file)")<<": "<<DEFAULT_INITFILE<<endl;

        // couldn't load default rc file, either
        if (!resource_manager.load(DEFAULT_INITFILE)) {
            cerr<<_FB_CONSOLETEXT(Fluxbox, CantLoadRCFile, "Failed to load database", "")<<": "<<DEFAULT_INITFILE<<endl;
            exit(1); // this is a fatal error for us
        }
    }

    // run updates here
    // I feel like putting this in a separate function for no apparent reason

    FbTk::Resource<int> config_version(resource_manager, 0,
            "session.configVersion", "Session.ConfigVersion");
    int old_version = *config_version;
    int new_version = run_updates(old_version, resource_manager);
    if (new_version > old_version) {
        // configs were updated -- let's save our changes
        config_version = new_version;
        resource_manager.save(rc_filename.c_str(), rc_filename.c_str());
        save_all_files();

#ifdef HAVE_SIGNAL_H
        // if we were given a fluxbox pid, send it a reconfigure signal
        if (fb_pid > 0)
            kill(fb_pid, SIGUSR2);
#endif // HAVE_SIGNAL_H

    }

    return 0;
}

static set<string> modified_files;
// we may want to put a size limit on this cache, so it doesn't grow too big
static map<string,string> file_cache;

// returns the contents of the file given, either from the cache or by reading
// the file from disk
string read_file(string filename) {
    // check if we already have the file in memory
    map<string,string>::iterator it = file_cache.find(filename);
    if (it != file_cache.end())
        return it->second;

    // nope, we'll have to read the file
    ifstream infile(filename.c_str());
    string whole_file = "";

    if (!infile) // failed to open file
        return whole_file;

    while (!infile.eof()) {
        string linebuffer;

        getline(infile, linebuffer);
        whole_file += linebuffer + "\n";
    }
    infile.close();

    file_cache[filename] = whole_file;
    return whole_file;
}

#ifdef NOT_USED
// remove the file from the cache, writing to disk if it's been changed
void forget_file(string filename) {
    map<string,string>::iterator cache_it = file_cache.find(filename);
    // check if we knew about the file to begin with
    if (cache_it == file_cache.end())
        return;

    // check if we've actually modified it
    set<string>::iterator mod_it = modified_files.find(filename);
    if (mod_it == modified_files.end()) {
        file_cache.erase(cache_it);
        return;
    }

    // flush our changes to disk and remove all traces
    ofstream outfile(filename.c_str());
    outfile << cache_it->second;
    file_cache.erase(cache_it);
    modified_files.erase(mod_it);
}
#endif // NOT_USED

// updates the file contents in the cache and marks the file as modified so it
// gets saved later
void write_file(string filename, string &contents) {
    modified_files.insert(filename);
    file_cache[filename] = contents;
}

// actually save all the files we've modified
void save_all_files() {
    set<string>::iterator it = modified_files.begin();
    set<string>::iterator it_end = modified_files.end();
    for (; it != it_end; ++it) {
        ofstream outfile(it->c_str());
        outfile << file_cache[it->c_str()];
    }
    modified_files.clear();
}
