#!python
# -*- coding: iso-8859-15 -*-
#
# $VER: makeall.py 1.5 (2005/12/14) © Guillaume ROGUEZ <yomgui1@gmail.com>
#
# Original Date:     2005/05/29
# Orginal Author:    Guillaume ROGUEZ <yomgui1@gmail.com>
#

import os, sys, shutil

from defaults import DefaultEnv
from utils import *

pjoin = os.path.join
exists = os.path.exists

sys.path.append(pjoin(sys.prefix, 'plat-' + os.name))

__all__ = [ 'handle_locale', 'DefaultEnv' ]

def handle_locale(name,
            srcpath,
            catpath=DefaultEnv['locale']['catpath'],
            language=DefaultEnv['locale']['language'],
            env=DefaultEnv):
    """Work as makefiles into _local/ directories."""

    if os.path.isabs(catpath):
        raise ValueError, "catpath must be relative to root path"

    locale = env['locale']
    dry = env['dryrun']
    cd = pjoin(srcpath, 'Catalog.cd')
    catpath = pjoin(env['root'], catpath, 'Catalogs', language, 'Feelin')
    if not exists(catpath):
        os.makedirs(catpath)

    # generate $ROOT/$catpath/$language/Feelin/FC_xxx.catalog file
    name = 'FC_'+name+'.catalog'
    catname = pjoin(catpath, name)
    rc = flexcat(catname, cd, pjoin(srcpath, language+'.ct'), env)
    if rc:
        raise CompileError(catname)

    # generate headers files
    h_templates_dir = locale['h_templates_dir']
    for file in os.listdir(h_templates_dir):
        name, ext = os.path.splitext(file)
        if ext == '.sd':
            header = pjoin(srcpath, name+'.h')
            rc = flexcat_header(header, cd, pjoin(h_templates_dir, file), env)
            if rc:
                raise CompileError(header)

##
#

class ScriptError(Exception):
    def __init__(self, cause, target=None):
        self.cause = cause
        self.target = target or ' '.join(argv) or action

    def __str__(self):
        return "Target '%s' failed: %s" % (self.target, self.cause)

class CompileError(ScriptError):
    def __init__(self, name, target=None):
        ScriptError.__init__(self, "compilation of '%s' failed" % name, target)
 

##
# Classes list by type

list_system    = 'AppServer Application Dataspace Decorator Display DOSNotify Family ModulesList Preference Thread WinServer Window'
list_support   = 'BitMap CorePNG Histogram ImageDisplay Palette Picture Render TextDisplay'
list_document  = 'Document CSSDocument HTMLDocument XMLDocument XMLObject XMLApplication'
list_gui       = 'Frame Area Balance Bar Crawler Cycle Decorator-Flatty Decorator-Shade DOSList Gauge Group Image List Listview Numeric Page PreferenceEditor PreferenceGroup Prop Radio RadioItem Scale Scrollbar Slider String Text'
list_adjust    = 'Adjust AdjustBrush AdjustColor AdjustFrame AdjustGradient AdjustImage AdjustPadding AdjustPen AdjustPreParse AdjustPicture AdjustRaster AdjustRGB AdjustScheme AdjustSchemeEntry'
list_pop       = 'PopBrush PopColor PopFile PopFont PopFrame PopHelp PopImage PopPicture PopPreParse PopScheme'
list_preview   = 'Preview PreviewColor PreviewFrame PreviewImage PreviewScheme'
list_dialog    = 'Dialog FileChooser FontChooser FontDialog'

list_removed = 'DOSList'

DefaultEnv['list'] = ' '.join([ list_system,
                                list_support,
                                list_document,
                                list_gui,
                                list_adjust,
                                list_pop,
                                list_preview,
                                list_dialog ]).split()

for x in list_removed.split():
    DefaultEnv['list'].remove(x)

if __name__ == '__main__':
    import sys
    from gcc import *

    strip_flags = '--remove-section .comment --strip-unneeded'

    usage_dict = {  'help': (('--help', '-h', '?'), "Print this text."),
                    
                    'dry': (('--dry-run', '--dry', '-n'),
                       "Enable dryrun mode.\nDon't change anything, just print commands"),
                    
                    'verbose': (('--verbose', '-v'), "Print all command executed."),
                    
                    'quiet': (('--quiet', '-q'), "Be really quiet.")}

    action_dict = { 'all': "Generate everything needed to use Feelin (default)",
    
                    'clean': """Remove all .o and .a files, except in excluded directories.
Default excluded directories are:\n\t""" + pjoin('$ROOT', os.sep.join([ 'Include', 'lib' ])),

                    'stripall': """recursive search in all paths given as argument or %s for all FCC libs
and run 'strip %s' on it.""" % (pjoin('$ROOT', 'Libs', 'Feelin'), strip_flags),
                    
                    'printcompile': "print command used to compile a .c file from a FCC directory.",
                    
                    'printlink': "print command used to link .o files to obtain a FCC library.",
                    
                    'include': "Make ppcinline's and the libfeelin_mos.a glue lib.",
                    
                    'library': "Make the feelin.library.",
                    
                    'prefs': "Make the Prefs program.",

                    'setup': "Make the Setup program."}
 
    def print_usage():
        print "usage: %s [options] action arguments...\noptions are:" % sys.argv[0]
        for value in usage_dict.values():
            text = value[1].splitlines() 
            l = list(value[0])
            l.sort()
            l.reverse()
            print '   %s %s' % (', '.join(l).ljust(30), text.pop(0))
            for line in text:
                print ' '*34+line
        
        print "\naction can be one of them:"
        for action, text in action_dict.iteritems():
            text = text.splitlines()
            print '   %s %s' % (action.ljust(30), text.pop(0))
            for line in text:
                print ' '*34+line
 

    ##
    # some defines

    maketime = os.path.getmtime(sys.argv[0]) 

    import copy
    env = copy.copy(DefaultEnv)
    env['argv'] = argv = sys.argv

    SRC  = 'Sources'
    ROOT = env['root']
    DST  = pjoin(ROOT, 'Libs', 'Feelin')
    OPT = '-mcpu=750 -mmultiple -mfused-madd -mstring'
    CFLAGS = '-noixemul -g -Wall -O3 -fomit-frame-pointer -fsigned-char -DAROS_ALMOST_COMPATIBLE -D__MORPHOS_NOSHAREDLIBS ' + OPT
    LFLAGS = '-noixemul ' + OPT
    LIB_LFLAGS = '-nostartfiles -Wl,-Map=/t/map ' + LFLAGS
    INCS = ['.', pjoin(ROOT, 'Include')]
    LIB_LFLAGS += ' -L' + pjoin(ROOT, 'Include', 'Lib')
    LFLAGS += ' -L' + pjoin(ROOT, 'Include', 'Lib')
    LIBS = ['feelin_mos', 'png_static', 'z_static', 'debug']
    

    ##
    # parsing arguments line

    argv = argv[1:]
    action_args = []
    action = None

    while True:
        try:
            arg = argv.pop(0)
        except IndexError:
            break

        # general option ?
        if arg in usage_dict['help'][0]:
            print_usage()
            sys.exit(0)
        elif arg in usage_dict['dry'][0]:
            env['dryrun'] = True
        elif arg in usage_dict['verbose'][0]:
            env['verbose'] = True
            env['quiet'] = False
        elif arg in usage_dict['quiet'][0]:
            env['quiet'] = True
            env['verbose'] = False
        elif action: # action option ?
            # action option ?

            # TODO
            
            # action arguments
            action_args.append(arg)
        else: # action
            if arg in action_dict.keys():
                action = arg.lower()
            else:
                # force default action
                action = 'all'
                argv.insert(0, arg)

    # Finalize options

    action = action or 'All'

    dry = env['dryrun']
    quiet = env['quiet']
    verbose = env['verbose']
    
    null = lambda x, *arg, **kw: None
    env['log_fct'] = log = ((not quiet) and log) or null
    del null

    def Link(name, sources, lflags=LFLAGS):
        objs = [ get_obj(src) for src in sources ]

        # is it needed to compile some files ?
        if objs:
            #objs = filter(lambda x: checkDepend(x, get_src(x)), objs)

            if objs:
                for obj in objs:
                    if compile(obj, get_src(obj),
                            cflags=CFLAGS,
                            includes=INCS,
                            env=env):
                        raise CompileError(obj, name)

            # check if destination directory exists. Make it if not.
            if not exists(os.path.dirname(name)):
                os.makedirs(name)

            # Linking the program
            if link(name, objs,
                    lflags=lflags,
                    libs=LIBS,
                    env=env):
                raise CompileError(name, os.path.basename(name))

        else:
            log('No object!')

    MakeProgram = Link
    MakeLibary = lambda name, sources: Link(name, sources, lflags=LIB_LFLAGS)

 
    ##
    # Let's go !

    if dry and verbose:
        print "[dry mode]\n"

    try:

        if action == 'clean':

            execute('Resident QUIET ADD C:Delete PURE', dry)

            exclude_dir = [ pjoin(ROOT, os.sep.join([ 'Include', 'lib' ])) ]
            for root, dirs, files in os.walk(ROOT):
                if root not in exclude_dir:
                    for file in filter(lambda x: os.path.splitext(x)[1] in [ '.o', '.a' ], files):
                        name = pjoin(root, file)
                        if not quiet:
                            print "Delete "+name
                        execute('Delete >NIL: '+name, dry)
            
            execute('Resident QUIET REMOVE Delete', dry)

        elif action == 'stripall':

            for path in action_args or [ pjoin('libs', 'feelin') ]:
                for root, dirs, files in os.walk(pjoin(ROOT, path)):
                    for file in filter(lambda x: os.path.splitext(x)[1] in [ '.elf' ], files):
                        name = pjoin(root, file)
                        if not quiet:
                            print "Strip "+name
                        execute('strip %s %s' % (strip_flags, name), dry)
     
        elif action == 'printcompile':

            env['verbose'] = True
            env['dryrun'] = True
            
            compile('dummy.o', 'dummy.c', cflags=CFLAGS, includes=INCS, env=env)

        elif action == 'printlink':

            env['verbose'] = True
            env['dryrun'] = True

            link('dummy', 'dummy.o', lflags=LIB_LFLAGS, libs=LIBS, env=env)

        elif action == 'include':

            incdir = pjoin(ROOT, 'Include')
            template = 'cvinclude.pl "--fd=%s%s_lib.fd" "--clib=%s%s_protos.h" '
            
            # gen glue lib 
            cmd = (template + '"--gluelib=%s"') % (pjoin(incdir, 'fd', 'feelin'), '',
                                                   pjoin(incdir, 'clib', 'feelin'), '',
                                                   pjoin(incdir, 'lib', 'libfeelin_mos.a'))
            if not quiet:
                log(cmd)
     
            execute(cmd, dry)

            # gen ppcinline's
            inlinedir = pjoin(incdir, 'ppcinline')
            def genInline(name):
                cmd = (template + '"--inlines=%s%s.h"') % (pjoin(incdir, 'fd', 'feelin'), name,
                                                       pjoin(incdir, 'clib', 'feelin'), name,
                                                       pjoin(inlinedir, 'feelin'), name)
                if not quiet:
                    log(cmd)
         
                execute(cmd, dry)
     
            if not exists(inlinedir):
                if not quiet:
                    log(inlinedir, pre="Making directory '", post="'")

                if not dry:
                    os.mkdir(inlinedir)

            genInline('')
            genInline('class')

        elif action == 'library':

            src_path = pjoin(ROOT, SRC, 'feelin')
            name = 'feelin.library.elf'

            if not exists(src_path):
                raise ScriptError("The path '"+src_path+"' doesn't exists", name)

            sources = map(lambda x: pjoin(src_path, x), filter(lambda x: x[-2:] == '.c', os.listdir(src_path)))
            lib_objs = [ get_obj(src) for src in sources ]

            if lib_objs:
                MakeLibary(pjoin(ROOT, 'Libs', name), lib_objs)

            else:
                log('No object!')

        elif action == 'prefs':

            MakeProgram(pjoin(ROOT, 'Prefs'), [ pjoin(ROOT, 'Prefs.c') ])

        elif action == 'setup':

            MakeProgram(pjoin(ROOT, 'Setup'), [ pjoin(ROOT, 'Setup.c') ])

        else: # 'all' action

            import re
            #from F_CreateFCCInfo_ppc import writeInfoFile

            env['list'] = action_args or env['list']

            prj_src = 'Project.c' 

            for name in env['list']:
                src_path = pjoin(ROOT, SRC, name)

                # defaults
                realname = None
                ver = 1
                rev = 0
                date = None
                cr = None

                olddir = os.getcwd()
                try:
                    os.chdir(src_path)
                
                except OSError:
                    print ScriptError("The path '"+src_path+"' doesn't exists", name)
                    print
                    continue

                try:
                    # is it a FCC directory ?
                    if exists(prj_src):

                        # Try to obtain '$VER:' string from the first line of prj_src file
                        _ver = re.compile(r'.*\$VER:\W*(?P<name>[a-zA-Z_-]*)\W+(?P<ver>\d+)(?:\.(?P<rev>\d+)[.\w]*)?(?:\W*\((?P<date>(?:\d|/)+)\))?(?:\W*(?P<cr>.*))?\W*$')
                        f = open(prj_src)
                        lines = f.readlines()
                        f.close()
                        mo = None
                        while lines and not mo:
                            mo = _ver.match(lines.pop(0))
                            if mo:
                                d = mo.groupdict()
                                realname = d['name'] or name.replace('/', '')
                                ver = d['ver']
                                rev = d['rev']
                                date = d['date']
                                cr = d['cr']

                        # no '$VER:' found... try the makefile 
                        if not realname:
                            _name = re.compile(r'^\W*NAME\W*=\W*(\w+)(\W*)|(#.*)$')
                            _ver = re.compile(r'^\W*VER\W*=\W*([0-9]+)(\W*)|(#.*)$')
                            _rev = re.compile(r'^\W*REV\W*=\W*([0-9]+)(\W*)|(#.*)$')
                            _cr = re.compile(r'^\W*AUTHOR\W*=\W*([0-9]+)(\W*)|(#.*)$')
                            _date = re.compile(r'^\W*DATE\W*=\W*([0-9/]+)(\W*)|(#.*)$')
                            try:
                                f = open('makefile')
                                for line in f.readlines():
                                    mo = _name.match(line)
                                    if mo:
                                        realname = mo.group(1)
                                    mo = _ver.match(line)
                                    if mo:
                                        ver = mo.group(1)
                                    mo = _rev.match(line)
                                    if mo:
                                        rev = mo.group(1)
                                    mo = _cr.match(line)
                                    if mo:
                                        cr = mo.group(1)
                                    mo = _date.match(line)
                                    if mo:
                                        date = mo.group(1)
                                f.close()
                            
                            except IOError: # no makefile
                                pass

                        if realname:
                            name = realname

                        log(' %s v%lu.%lu ' % (name, int(ver), int(rev)), pre='='*6, post='='*6+'\n')
                        
                        lib_file = pjoin(DST, name+'.fc.elf')

                        # handle _locale directory
                        if exists('_locale'):
                            handle_locale(name, '_locale', env=env)

                        sources = [ prj_src ]
                        sources.extend(filter(lambda x: (x != prj_src) and (x[-2:] == '.c'), os.listdir(os.curdir)))
                        
                        MakeLibary(lib_file, sources)

                        log(name, post=" is compiled\n")

                    else:
                        print "\n"+name+" is not a FCC source directory."
                
                finally:
                    os.chdir(olddir)

            if verbose:
                print "*** Target(s) '%s' done ***" % ' '.join(argv) or action

    except (ScriptError, CompileError), error:
        print '\n' + str(error)
        sys.exit(10)
 
del pjoin
