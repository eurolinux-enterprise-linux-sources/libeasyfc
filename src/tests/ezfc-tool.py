#! /usr/bin/env python
# ezfc-tool.py
# Copyright (C) 2011-2013 Akira TAGOH

# Authors:
#   Akira TAGOH  <akira@tagoh.org>

# This library is free software: you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation, either
# version 3 of the License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from gi.repository import Easyfc
from gi.repository import GLib
import gi
import os.path
import sys
import getopt
import locale

config_priority = 0
config_extra_name = None

def __cmd_add_cb(argv):
    opts, args = getopt.getopt(argv, 'a:hl:s:',
                               ['alias=', 'help', 'lang=', 'subst=', 'no-load',
                                'hinting=', 'autohint=', 'antialias=',
                                'embeddedbitmap=', 'rgba=', 'hintstyle='])
    opts.append(('', None)) # just to ensure entering the loop
    loadconf = True
    alias = None
    lang = None
    hinting = None
    autohint = None
    antialias = None
    embeddedbitmap = None
    rgba = None
    hintstyle = None
    for o, a in opts:
        if o in ('-h', '--help') or len(args) != 1:
            print('Usage: %s add [options] <family>' % os.path.basename(sys.argv[0]))
            print('Options:')
            print('  -a or --alias=ALIAS        Set ALIAS as the alias font for family')
            print('  -h or --help               Show this message')
            print('  -l or --lang=LANG          Set LANG as the language to be added')
            print('  -s or --subst=SUBST[,...]  Set SUBST as the substitute fonts for family')
            print('  --no-load                  Do not load the configuration file')
            print('  --hinting=<bool>           Set a boolean value for hinting')
            print('  --autohint=<bool>          Set a boolean value for auto-hinting')
            print('  --antialias=<bool>         Set a boolean value for antialiasing')
            print('  --embeddedbitmap=<bool>    Set a boolean value for embeddedbitmap')
            print('  --rgba=<const>             Set rgba')
            print('  --hintstyle=<const>        Set hintstyle')
            sys.exit()
        elif o in ('-a', '--alias'):
            alias = a
        elif o in ('-l', '--lang'):
            lang = a
        elif o in ('-s', '--subst'):
            subst = a
        elif o == '--no-load':
            loadconf = False
        elif o == '--hinting':
            if a.lower() == "true" or a.lower() == "1" or a.lower() == "yes":
                hinting = True
            else:
                hinting = False
        elif o == '--autohint':
            if a.lower() == "true" or a.lower() == "1" or a.lower() == "yes":
                autohint = True
            else:
                autohint = False
        elif o == '--antialias':
            if a.lower() == "true" or a.lower() == "1" or a.lower() == "yes":
                antialias = True
            else:
                antialias = False
        elif o == '--embeddedbitmap':
            if a.lower() == "true" or a.lower() == "1" or a.lower() == "yes":
                embeddedbitmap = True
            else:
                embeddedbitmap = False
        elif o == '--rgba':
            map = {
                'unknown': 0,
                'rgb': 1,
                'bgr': 2,
                'vrgb': 3,
                'vbgr': 4,
                'none': 5,
                }
            if map.has_key(a.lower()):
                rgba = map[a.lower()]
        elif o == '--hintstyle':
            map = {
                'hintnone': 1,
                'hintslight': 2,
                'hintmedium': 3,
                'hintfull': 4,
                }
            if map.has_key(a.lower()):
                hintstyle = map[a.lower()]

    if (hinting != None or autohint != None or antialias != None or embeddedbitmap != None or rgba != None or hintstyle != None) and alias != None:
        print("E: --alias can't be used with --hinting, --autohint, --antialias, --embeddedbitmap, --rgba or --hintstyle")
        sys.exit()

    config = Easyfc.Config()
    config.set_priority(config_priority)
    if config_extra_name != None:
        config.set_name(config_extra_name)
    if loadconf:
        try:
            config.load()
        except GLib.GError as e:
            if e.domain == 'ezfc-error-quark' and e.code == 7:
                pass
            else:
                raise

    if alias != None:
        eza = Easyfc.Alias.new(alias)
        eza.set_font(args[0])
        config.add_alias(lang, eza)
        msg = '%s has been added as the alias of %s for %s' % (args[0], alias, lang)
    elif subst != None:
        for n in subst.split(','):
            ezf = Easyfc.Font()
            ezf.set_family(n)
            config.add_subst(args[0], ezf)
        msg = '%s has been added as the subst of %s' % (subst, args[0])
    else:
        ezf = Easyfc.Font()
        ezf.set_family(args[0])
        amsg = []
        if hinting != None:
            ezf.set_hinting(hinting)
            amsg.append('hinting is %s for %s' % ('enabled' if hinting else 'disabled', args[0]))
        if autohint != None:
            ezf.set_autohinting(autohint)
            amsg.append('auto-hinting is %s for %s' % ('enabled' if autohint else 'disabled', args[0]))
        if antialias != None:
            ezf.set_antialiasing(antialias)
            amsg.append('anti-aliasing is %s for %s' % ('enabled' if antialias else 'disabled', args[0]))
        if embeddedbitmap != None:
            ezf.set_embedded_bitmap(embeddedbitmap)
            amsg.append('embedded bitmap is %s for %s' % ('enabled' if embeddedbitmap else 'disabled', args[0]))
        if rgba != None:
            ezf.set_rgba(rgba)
            amsg.append('rgba is set to %s for %s' % (rgba, args[0]))
        if hintstyle != None:
            ezf.set_hintstyle(hintstyle)
            amsg.append('hintstyle is set to %s for %s' % (hintstyle, args[0]))

        msg = '\n'.join(amsg)
        config.add_font(ezf)

    config.save()
    print(msg)

def __cmd_remove_cb(argv):
    opts, args = getopt.getopt(argv, 'ahl:ps:', ['alias', 'help', 'lang=', 'prop', 'subst='])
    opts.append(('', None)) # just to ensure entering the loop
    mode = None
    lang = None
    subst = None
    for o, a in opts:
        if o in ('-h', '--help') or len(args) != 1:
            print('Usage: %s remove [options] <family>' % os.path.basename(sys.argv[0]))
            print('Options:')
            print('  -a or --alias        Remove the alias')
            print('  -p or --prop         Remove the font properties')
            print('  -s or --subst=SUBST  Remove SUBST from the substitute font')
            print('  -l or --lang=LANG    Set LANG as the language to be removed. imply -a option')
            print('  -h or --help         Show this message')
            sys.exit()
        elif o in ('-a', '--alias'):
            mode = 'alias'
        elif o in ('-p', '--prop'):
            mode = 'prop'
        elif o in ('-s', '--subst'):
            mode = 'subst'
            subst = a
        elif o in ('-l', '--lang'):
            if mode != 'alias':
                print("E: --lang has to be set with --alias option")
                sys.exit()
            else:
                lang = a

    config = Easyfc.Config()
    config.set_priority(config_priority)
    if config_extra_name != None:
        config.set_name(config_extra_name)
    try:
        config.load()
    except GLib.GError as e:
        if e.domain == 'ezfc-error-quark' and e.code == 7:
            print('E: no configuration file available')
            sys.exit()
        else:
            raise

    if mode == 'alias':
        if not config.remove_alias(lang, args[0]):
            msg = 'Unable to remove %s from %s' % (args[0], lang)
        else:
            msg = '%s has been removed from %s' % (args[0], lang)
    elif mode == 'prop':
        if not config.remove_font(args[0]):
            msg = 'Unable to remove the properties for %s' % args[0]
        else:
            msg = 'Properties has been removed for %s' % args[0]
    elif mode == 'subst':
        if not config.remove_subst(args[0], subst):
            msg = 'Unable to remove %s from %s' % (subst, args[0])
        else:
            msg = '%s has been removed from %s' % (subst, args[0])
    else:
        print("E: Unexpected operation mode: %s" % mode)
        sys.exit()
    config.save()
    print(msg)

def __cmd_show_cb(argv):
    opts, args = getopt.getopt(argv, 'afh', ['alias', 'feature', 'help'])
    opts.append(('', None)) # just to ensure entering the loop
    mode = None
    for o, a in opts:
        if o in ('-h', '--help') or len(args) > 2:
            print('Usage: %s show [options] <[lang] [alias]|<fontname>>' % os.path.basename(sys.argv[0]))
            print('Options:')
            print('  -a or --alias   Display aliases')
            print('  -f or --feature Display font features')
            print('  -h or --help    Show this message')
            sys.exit()
        elif o in ('-a', '--alias'):
            mode = 'alias'
        elif o in ('-f', '--feature'):
            mode = 'feature'

    config = Easyfc.Config()
    config.set_priority(config_priority)
    if config_extra_name != None:
        config.set_name(config_extra_name)
    try:
        config.load()
    except GLib.GError as e:
        if e.domain == 'ezfc-error-quark' and e.code == 7:
            if mode != 'feature':
                print('E: no configuration file available')
                sys.exit()
        else:
            raise

    if mode == 'alias':
        if len(args) == 0:
            config.dump()
        elif len(args) == 1:
            l = config.get_aliases(args[0])
            if len(l) == 0:
                print('E: no aliases defined for %s' % args[0])
            else:
                for a in l:
                    print('%s: %s' % (a.get_name(), a.get_font()))
        else:
            l = config.get_aliases(args[0])
            if len(l) == 0:
                print('E: no aliases defined for %s' % args[0])
            else:
                if args[1] == 'sans':
                    alias = 'sans-serif'
                else:
                    alias = args[1]
                for a in l:
                    if a.get_name() == alias:
                        print(a.get_font())
                        sys.exit()
                print('E: no such alias for %s: %s' % (args[0], args[1]))
    elif mode == 'feature':
        if len(args) == 0:
            print('E: no font specified to look up')
            sys.exit()
        ezf = Easyfc.Font()
        ezf.set_family(args[0])
        print('Available features: ',)
        for n in ezf.get_available_features():
            print(n,)
        print('')
        print('Configured features: ',)
        for n in ezf.get_features():
            print(n,)
        print('')

def __cmd_fonts_cb(argv):
    opts, args = getopt.getopt(argv, 'a:l:h', ['alias=', 'lang=', 'help'])
    opts.append(('', None)) # just to ensure entering the loop
    alias = None
    lang = None
    for o, a in opts:
        if o in ('-h', '--help'):
            print('Usage: %s fonts [options]' % os.path.basename(sys.argv[0]))
            print('Options:')
            print('  -h or --help         Show this message')
            print('  -a or --alias=ALIAS  Set ALIAS as the alias font name for query')
            print('  -l or --lang=LANG    Set LANG as the language for query')
            sys.exit()
        elif o in ('-a', '--alias'):
            alias = a
        elif o in ('-l', '--lang'):
            lang = a

    Easyfc.init()
    fonts = Easyfc.Font.get_list(lang, alias, False)
    print('%s (lang=%s):' % (alias, lang))
    for f in fonts:
        print('  %s' % f)

    Easyfc.finalize()

def __cmd_help_cb(argv):
    print('Usage: %s [global options] <command> ...' % os.path.basename(sys.argv[0]))
    print('Commands:')
    print('  %s' % ', '.join(commands.keys()))
    print('Global Options:')
    print('  -p or --priority=NUM   Priority number in the filename.')
    print('  -n or --name=STRING    A extra name used in the filename.')
    sys.exit()

commands = {
    "add":__cmd_add_cb,
    "remove":__cmd_remove_cb,
    "show":__cmd_show_cb,
    "fonts":__cmd_fonts_cb,
    "help":__cmd_help_cb,
}

locale.setlocale(locale.LC_ALL, '')

if len(sys.argv) == 1:
    __cmd_help_cb(sys.argv[1:])

opts, args = getopt.getopt(sys.argv[1:], 'p:n:h', ['alias=', 'lang=', 'help'])
for o, a in opts:
    if o in ('-h', '--help'):
        __cmd_help_cb(sys.argv[2:])
    elif o in ('-p', '--priority'):
        config_priority = int(a)
    elif o in ('-n', '--name'):
        config_extra_name = a

if args[0] in commands.keys():
    # getopt is broken when arguments contains a space.
    pos = sys.argv.index(args[0])
    commands[args[0]](args[pos:])
else:
    __cmd_help_cb(sys.argv[2:])
