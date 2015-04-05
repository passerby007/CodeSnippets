#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import time
import re
import subprocess
import ConfigParser

kConfigName = 'cache'
kConfigMain = 'main'
kConfigSite = 'site'

kPostEditor = 'atom'
kVCSEditor = 'GitHub'
kPostSuffix = '.markdown'

kPostTitle = ''
kPostDate = ''
kPostTime = ''
kPostFileTitle = ''
kSitePath = ''
kPostPath = ''

def save_config():
    if kSitePath is '': return

    thispath = os.path.realpath(__file__)
    thisdir = os.path.dirname(thispath)

    configPath = os.path.join(thisdir, kConfigName)

    config = ConfigParser.ConfigParser()
    config.add_section(kConfigMain)
    config.set(kConfigMain, kConfigSite, kSitePath)

    f = open(configPath, 'w')
    config.write(f)
    f.close()
    return

def load_config():
    thispath = os.path.realpath(__file__)
    thisdir = os.path.dirname(thispath)

    configPath = os.path.join(thisdir, kConfigName)

    config = ConfigParser.ConfigParser()
    config.read(configPath)
    site = config.get(kConfigMain , kConfigSite)

    handle_site(site)
    return

def set_default_date():
    global kPostDate
    global kPostTime

    kPostDate = time.strftime('%Y-%m-%d')   # Year-Month-Day
    kPostTime = time.strftime('%H:%M:%S')   # Hour:Min:Sec
    return

def should_replace(b):
    c = ord(b)
    if c >= 128: return False   # non-ascii codec

    return not chr(c).isalnum()

def handle_site(path):
    global kSitePath

    valid = os.path.isdir(path)
    if valid: kSitePath = path

    return valid

def handle_title(title):
    """
    only numbers and letters are kept in the title, others will be replaced by '-'
    :param title:
    :return:
    """
    global kPostTitle
    global kPostFileTitle

    if title == '': return False

    kPostTitle = title

    r = 0
    n = len(title)
    minus = 0

    while r < n:
        if should_replace(title[r]):
            if minus == 0:
                kPostFileTitle += '-'
                minus = 1
                pass
        else:
            kPostFileTitle += title[r]

            minus = 0
            pass

        r += 1

    kPostFileTitle = kPostFileTitle.strip('-').lower()

    return kPostFileTitle is not ''

def handle_date(date):
    global kPostDate
    global kPostTime

    parts = date.strip(' ').split(' ')

    n = len(parts)
    if n is not 1 and n is not 2 : return False

    # handle the date
    d = parts[0]

    pattern = '([1-9][0-9]{0,3})-((0[1-9])|(1[0-2]))-((0[1-9])|([1-2][0-9])|(3[0-1]))'
    m = re.match(pattern, d)    # yyyy-mm-dd

    valid = (m is not None) and (len(m.group(0)) == len(d))
    if valid:
        kPostDate = d

    if (not valid) or (n is not 2): return valid

    # handle the time
    t = parts[1]
    pattern = '([0-1][0-9]|2[0-3])(:[0-5][0-9]){1,2}'
    m = re.match(pattern, t)    # HH:MM(:SS)

    valid = (m is not None) and (len(m.group(0)) == len(t))
    if valid:
        kPostTime = t
    else:
        print('Warning: ignore invalid time parameter. Got: ' + t)
        kPostTime = ''

    return True

def print_usage():
    print('JekyllPoster Usage: \n'
          'python JekyllPoster.py -t title [-d date] [-s site]\n'
          '    -t title of the new post. utf-8 encoded.\n'
          '    -d [optional] creation date of the new post, default by today.\n'
          '       the format is supposed to be "YYYY-MM-DD hh:mm:ss". and the\n'
          '       time part (hh:mm:ss) is not necessary.\n'
          '    -s [conditional] the Jekyll site path. If no specific site path \n'
          '       is assigned, the program will try to read the cache file.\n'
          '       it\'s supposed to contain the \'_posts\' folder, otherwise\n'
          '       a new _posts folder will be automatically generated.\n'
    )
    return

def check_parameters():
    global kPostTitle
    global kPostDate
    global kSitePath

    if kPostTitle == '':
        print('Fail: a post title is required!')
        print_usage()
        return False

    if kSitePath == '':
        load_config()

    if kSitePath == '':
        print('Fail: the Jekyll site path is requried, and no avaible cached path is found!')
        print_usage()
        return False

    if kPostDate == '':
        set_default_date()

    return True

def find_valid_name(folder, name, suffix):
    part = os.path.join(folder, name)

    target = part + suffix

    if not os.path.exists(target): return target

    idx = 1
    idxMax = 10000
    while idx < idxMax:
        target = part + '-' + str(idx) + suffix
        if not os.path.exists(target): return target

        idx += 1
        pass

    return ''

def build_post():
    global kPostPath

    basename = kPostDate + '-' + kPostFileTitle
    sitepost = os.path.join(kSitePath, '_posts')

    if not os.path.exists(sitepost):
        os.makedirs(sitepost)

    if not os.path.isdir(sitepost):
        print("Error: invalid site post folder! %s" % sitepost)
        return False

    fullpath = find_valid_name(sitepost, basename, kPostSuffix)
    if fullpath == '':
        print("Error: cannot create the post at the given path: " + sitepost)
        return False

    header = ('---\n'
              'layout: post\n'
              'title: %s\n'
              'date: %s %s\n'
              'categories: \n'
              'tags: \n'
              '---\n'
             ) % (kPostTitle, kPostDate, kPostTime)

    try:
        f = open(fullpath, 'w')
        f.write(header)
        f.close()
    except:
        print('Error: cannot open and write content to the post!\n' + fullpath)
        return False

    print('New post: %s %s %s\n'
          'Path: %s' %
          (kPostTitle, kPostDate, kPostTime, fullpath)
    )

    kPostPath = fullpath
    save_config()
    return True

def post_build():

    PostCommands = [
        ['wait', kPostEditor, '-w', kPostPath],
        ['detatch', 'open', '-a', kVCSEditor]
    ]

    cmdName = ''
    try:
        for cmd in PostCommands:
            state = cmd[0]
            cmdName = cmd[1]
            print cmd

            code = -1
            if state == 'detatch':
                code = subprocess.call(cmd[1:])
            elif state == 'wait':
                code = subprocess.check_call(cmd[1:])

            if code is not 0: raise
    except:
        print('Error: cannot execute the given command: ' + cmdName)
        return False

    return True

def _main():
    #print sys.argv

    # ensure the kPostEditor app could be possibly found
    if sys.platform is not 'win32':
        os.environ['PATH'] += ':/usr/local/bin'

    i = 1
    n = len(sys.argv)

    while i <= n - 1:
        arg = sys.argv[i]
        ret = True

        value = sys.argv[i+1]
        if arg == '-d':
            ret = handle_date(value)
        elif arg == '-t':
            ret = handle_title(value)
        elif arg == '-s':
            ret = handle_site(value)
            pass

        if ret is not True:
            print('bad parameter! %s: %s' % (arg, value))
            print_usage()
            return -1

        i += 2
        pass

    if not check_parameters(): return -2
    if not build_post(): return -3
    if not post_build(): return -4

    return 0

if __name__ == '__main__':
    exit(_main())
