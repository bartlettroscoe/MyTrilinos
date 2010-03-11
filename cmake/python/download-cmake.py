#!/usr/bin/env python

#
# General scripting support
#
# NOTE: Included first to check the version of python!
#

from GeneralScriptSupport import *

from CMakeBinaries import *

import re
import shutil
import string
import sys
import tarfile
import urllib
import zipfile


#
# Detailed documentation
#

usageHelp = r"""download-cmake.py [--install-dir=$HOME/cmake ...]

Script that downloads and installs one of the verified versions of CMake for
building Trilinos. The verified versions of CMake and their available
installers are indicated by the files CMakeVersions.py and CMakeBinaries.py.

There are multiple verified versions: you can choose which one you want to use
by passing the --installer-type argument to this script.

Valid values for --installer-type are:

  'min' - minimum required version of CMake that can build Trilinos
  'release' - default / recommended version, may be same as min
  'rc' - latest release candidate, may be same as release
  'dev' - latest development build

By default, if you just type:

   $ SOME_DIR/download-cmake.py

then the directory download_area will get created in the local working
directory. It will contain the extracted install tree of a pre-built
binary CMake suitable for this platform. That extracted install tree may
be used directly from the download_area directory, or you may move or
copy it to any location of your choosing.

This script will also pull down the list of available installers from the CMake
web site and detect the latest available installer for the v2.8 and development
builds. After running the detection phase of the script, a new CMakeVersions.py
is written in the download_area directory. Periodically as needed, this
generated CMakeVersions.py should be committed as the official CMakeVersions.py
file in the same directory with this script.

You can control various parts of the process with the options (see below). Call
this script with -h or --help for a list of possible options.

Example using the --install-dir option:

  $ SOME_DIR/download-cmake.py --install-dir=$HOME/cmake

This usage would install CMake and the other executables in $HOME/cmake/bin.
NOTE: You will have to update your PATH variable to include whatever directory
you choose to install CMake in.

NOTE: If you need to use sudo to install in some place that requires root
privileges, or if you need to install to a directory that already exists, do:

  $ ./download-cmake.py --skip-install [other options]
  $ cd download_area
  $ sudo cp -r cmake-2.8.0-Linux-i386/ /usr/local

After you have done a successful install, you may remove the downloaded files:

  $ rm -r download_area

Enjoy CMake!

"""


#
# Read in the commandline arguments
#

from optparse import OptionParser

clp = OptionParser(usage=usageHelp)

clp.add_option(
  "--all-platforms", dest="allPlatforms", action="store_true", default=False,
  help="Download and/or extract tarballs for all platforms (default = just this platform)" )

clp.add_option(
  "--install-dir", dest="installDir", type="string", default="/usr/local",
  help="The install directory for CMake (default = /usr/local)." )

clp.add_option(
  "--installer-type", dest="installerType", type="string", default="release",
  help="Which CMake installer: min, release, rc, or dev (default = release)." )

clp.add_option(
  "--skip-detect", dest="skipDetect", action="store_true", default=False,
  help="Skip detecting the latest available builds" )

clp.add_option(
  "--skip-download", dest="skipDownload", action="store_true", default=False,
  help="Skip the download step" )

clp.add_option(
  "--skip-extract", dest="skipExtract", action="store_true", default=False,
  help="Skip the extract step" )

clp.add_option(
  "--skip-install", dest="skipInstall", action="store_true", default=False,
  help="Skip the install step" )

(options, args) = clp.parse_args()


# "Latest available CMake build" is defined as the most recent build on the
# server in vdir with pre-built binary tarballs available for Mac, Linux and
# Windows.
#
def DetectLatestCMakeBuilds(basedir, baseurl, vdir):
  url = ''.join([baseurl, "/", vdir, "/?C=M;O=D"])
  filename = ''.join([basedir, "/CMake_", vdir, ".html"])

  createDir(basedir)

  print "Querying " + url + "..."
  urllib.urlretrieve(url, filename)

  lines = []
  regex = re.compile(
    "href.*cmake-[0-9.]+(-rc[0-9]+)*-(Darwin-universal.tar.gz|Linux-i386.tar.gz|win32-x86.zip)"
  )

  f = open(filename)
  alllines = f.readlines()
  for line in alllines:
    if regex.search(line):
      lines.append(line)

  count = 0
  found = 0
  version_iterator = ""

  versionRegEx = re.compile(r'.*-([0-9.]+-rc[0-9]+|[0-9.]+)-.*')
  dateRegEx = re.compile(r'^[0-9].[0-9].([0-9.]+-rc[0-9]+|[0-9.]+)$')
  hrefRegEx = re.compile(r'^.*href="([^"]+)".*$')

  for line in lines:
    version = versionRegEx.match(line).group(1)

    if version == "" or version == line:
      print "error: line does not match version extraction regex"
      print " line: [" + line + "]"
      exit(1)

    date = dateRegEx.match(version).group(1)

    # l, m, w == found an installer for Linux, Mac, Windows respectively
    # When encountering a new version, reset back to zeroes...
    #
    if(version_iterator != version):
      version_iterator = version
      l = 0
      m = 0
      w = 0
      lhref = ""
      mhref = ""
      whref = ""

    href = hrefRegEx.match(line).group(1)

    if re.search('Linux', line) != None:
      lhref = href
      l = 1
    elif re.search('Darwin', line) != None:
      mhref = href
      m = 1
    elif re.search('win32', line) != None:
      whref = href
      w = 1
    else:
      print "error: unexpected non-matching line"
      exit(1)

    count = count + 1

    if l == 1 and m == 1 and w == 1:
      found = 1
      print "Detected latest available CMake " + vdir + " build: " + version
      break

  if not found:
    print "error: could not find a " + vdir + " version with all 3 platforms available"
    return ()

  return (('linux2', lhref, version), ('darwin', mhref, version), ('win32', whref, version))


def Download(basedir, url):
  cmps = url.rsplit("/", 1)
  href = cmps[1]
  filename = ''.join([basedir, "/", href])

  print 'Downloading ' + href + '...'

  urllib.urlretrieve(url, filename)


def Extract(basedir, url):
  cmps = url.rsplit("/", 1)
  href = cmps[1]
  filename = ''.join([basedir, "/", href])

  print 'Extracting ' + href + '...'

  if href[-4:] == ".zip":
    if sys.version < '2.6':
      if sys.platform == 'win32':
        print "error: cannot extract zip files on win32 with older python < 2.6"
      else:
        print "warning: avoiding zipfile.extractall on older python < 2.6"
        print "         skipping this extraction..."
    else:
      z = zipfile.ZipFile(filename)
      z.extractall(basedir)
      z.close()
  else:
    if sys.version < '2.6':
      if sys.platform == 'win32':
        print "error: cannot extract tar files on win32 with older python < 2.6"
      else:
        print "warning: avoiding tarfile.extractall on older python < 2.6"
        print "         trying command line tar instead..."
        origDir = os.getcwd()
        echoChDir(basedir)
        echoRunSysCmnd("tar -xzf " + href)
        echoChDir(origDir)
    else:
      t = tarfile.open(filename)
      t.extractall(basedir)
      t.close()


def Install(basedir, url):
  cmps = url.rsplit("/", 1)
  href = cmps[1]

  if href[-4:] == ".zip":
    href = href[:-4]
  elif href[-7:] == ".tar.gz":
    href = href[:-7]

  dirname = ''.join([basedir, "/", href])

  print 'Installing ' + href + '...'
  print '  src dir: [' + dirname + ']'
  print '  dst dir: [' + options.installDir + ']'

  if sys.platform == 'win32':
    if os.path.exists(options.installDir):
      print "error: --install-dir '" + options.installDir + "' already exists - remove it or rename it and try again -- or manually copy the source directory '" + dirname + "' to the final installation location..."
      exit(1)

    shutil.copytree(dirname, options.installDir)
  else:
    # avoid the "copytree doesn't work if dir already exists" problem by using
    # the sys command "cp"
    createDir(options.installDir)
    echoRunSysCmnd("cp -r " + dirname + "/ " + options.installDir)


def DownloadForPlatform(p):
  if options.allPlatforms:
    return True
  if p == sys.platform:
    return True
  return False


def PrintDetectedDownloads(detected):
  print ""
  print "Detected CMake downloads available:"

  sorted_keys = detected.keys()
  sorted_keys.sort()

  detected_urls = list()

  for k in sorted_keys:
    for v in detected[k]:
      if DownloadForPlatform(v[0]):
        detected_urls.append(cmake_baseurl + "/" + k + "/" + v[1])

  for u in detected_urls:
    print "[" + u + "]"


def PrintVerifiedDownloads():
  print ""
  print "Verified CMake downloads:"

  verified_urls = list()

  for v in cmake_min_binaries:
    if DownloadForPlatform(v[0]):
      verified_urls.append(v[1])

  for v in cmake_release_binaries:
    if DownloadForPlatform(v[0]):
      verified_urls.append(v[1])

  for v in cmake_rc_binaries:
    if DownloadForPlatform(v[0]):
      verified_urls.append(v[1])

  for v in cmake_dev_binaries:
    if DownloadForPlatform(v[0]):
      verified_urls.append(v[1])

  for u in verified_urls:
    print "[" + u + "]"


# Read file "CMakeVersions.py" from the same directory that this script lives
# in. Then write an auto_updated edition of CMakeVersions.py in download_dir
# based on the detected latest available build for each auto_update vdir entry.
#
# Each line that matches the "auto_update" regex will be updated using the
# latest data in the "detected" dictionary. Other lines will be written out
# verbatim.
#
def ReadWriteCMakeVersionsFile(download_dir, detected):
  rfname = os.path.dirname(os.path.abspath(__file__)) + "/CMakeVersions.py"

  fr = open(rfname)
  lines = fr.readlines()

  wfname = download_dir + "/CMakeVersions.py"
  fw = open(wfname, "w")

  regex = re.compile(
    "cmake_version_(.*) = \"(.*)\" # auto_update ([^ \t\n]*)"
  )

  for line in lines:
    if regex.search(line):
      #fw.write("# original: ")
      #fw.write(line)

      type = regex.match(line).group(1)
      vdir = regex.match(line).group(3)

      binaries = detected[vdir]
      binary_0 = binaries[0]
      version = binary_0[2]

      auto_update_line = "cmake_version_" + type + " = \"" + version + "\" # auto_update " + vdir + "\n"

      fw.write(auto_update_line)
    else:
      fw.write(line)

  print ""
  print "Wrote new '" + wfname + "' -- copy to '" + rfname + "' (if different) to use newly detected installers."


#
# The main script
#

download_dir = "download_area"

binaries = None
if options.installerType == 'min':
  binaries = cmake_min_binaries
if options.installerType == 'release':
  binaries = cmake_release_binaries
if options.installerType == 'rc':
  binaries = cmake_rc_binaries
if options.installerType == 'dev':
  binaries = cmake_dev_binaries
if binaries == None:
  print "error: unknown --installer-type: [" + options.installerType + "]"
  exit(1)


print ""
print ""
print "A) Detect the latest available builds of CMake ..."
print "    (requires network access to www.cmake.org)"
print ""

if options.skipDetect:
  print "Skipping on request ..."
else:
  detected = dict()

  for vdir in cmake_vdirs:
    detected[vdir] = DetectLatestCMakeBuilds(download_dir, cmake_baseurl, vdir)

  PrintDetectedDownloads(detected)

  PrintVerifiedDownloads()

  ReadWriteCMakeVersionsFile(download_dir, detected)


print ""
print ""
print "B) Download CMake for --installer-type '" + options.installerType + "' ..."
print "    (requires network access to www.cmake.org)"
print ""

if options.skipDownload:
  print "Skipping on request ..."
else:
  for binary in binaries:
    if DownloadForPlatform(binary[0]):
      Download(download_dir, binary[1])


print ""
print ""
print "C) Extract the CMake install tree ..."
print ""

if options.skipExtract:
  print "Skipping on request ..."
else:
  for binary in binaries:
    if DownloadForPlatform(binary[0]):
      Extract(download_dir, binary[1])


print ""
print ""
print "D) Install (copy the CMake install tree) ..."
print ""

if options.skipInstall:
  print "Skipping on request ..."
else:
  for binary in binaries:
    if binary[0] == sys.platform:
      Install(download_dir, binary[1])