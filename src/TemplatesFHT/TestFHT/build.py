# coding: cp1251
import os
import sys
import time
import subprocess
import argparse
from functools import reduce
class build:
  gen_win32 = 'cmake .. -G "Visual Studio 15 2017"'
  gen_win64 = 'cmake .. -G "Visual Studio 15 Win64"'
  gen_Ninja = 'cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release'
  build_Ninja = 'cmake --build . --parallel 16'
  build_Win = 'cmake --build . --config Release --parallel 16'
class bcolors:
  HEADER = '\033[95m'
  OKBLUE = '\033[94m'
  OKGREEN = '\033[92m'
  WARNING = '\033[93m'
  FAIL = '\033[91m'
  ENDC = '\033[0m'
  BOLD = '\033[1m'
  UNDERLINE = '\033[4m'
def createParser ():
  parser = argparse.ArgumentParser(add_help=True)
  parser.add_argument ('-b', '--build', nargs='?', const ='ninja')
  return parser
def Win():
  if os.name != 'nt':
    subprocess.call('apt-get install build-essential autoconf libtool pkg-config -y', shell=True)
    subprocess.call('apt-get install libgflags-dev libgtest-dev -y', shell=True)
    subprocess.call('apt-get install clang libc++-dev -y', shell=True)
    return
  os.environ['CC']='cl.exe'
  os.environ['CXX']='cl.exe'
  #outFromProcessCall(r'call "' +os.environ["VS2017INSTALLDIR"]+ r'\VC\Auxiliary\Build\vcvarsall.bat" x64')
def outFromProcess(args):
  callprocess = subprocess.Popen(args, stdout=subprocess.PIPE, shell=True)
  if callprocess.wait():
    print (bcolors.FAIL + "fatal"+ bcolors.ENDC)
  return callprocess.communicate()
def outFromProcessCall(args):
  callprocess = subprocess.call(args, shell=True)
  print(args)
  return callprocess
def secondsToStr(t):
  return "%d:%02d:%02d.%03d" % reduce(lambda ll,b : divmod(ll[0],b) + ll[1:],[(t*1000,),1000,60,60])
def main(namespace):
  start_time = time.time()
  if os.path.isdir(r"build") != True:
    os.mkdir(r"build")
  os.chdir('./build')
  if namespace.build == 'ninja':
    if os.name == 'nt':
      Win()
    outFromProcessCall(build.gen_Ninja)
    outFromProcessCall(build.build_Ninja)
  elif namespace.build == 'vs32':
    Win()
    outFromProcessCall(build.gen_win32)
    outFromProcessCall(build.build_Win)
  elif namespace.build == 'vs64':
    Win()
    outFromProcessCall(build.gen_win64)
    outFromProcessCall(build.build_Win)
  print (bcolors.OKBLUE + '\n\r---> Build Time ---> ' + secondsToStr(time.time() - start_time) + bcolors.ENDC)
  return False
if __name__ == "__main__":
  if len(sys.argv) < 2: 
    print (bcolors.FAIL + "---> !!! Not found parameters of build use '-b' + 'ninja|vs32|vs64  !!!" + bcolors.ENDC)
    sys.exit(1)
  parser = createParser()
  namespace = parser.parse_args(sys.argv[1:])
  if namespace.build != 'vs32' and namespace.build != 'vs64' and namespace.build != 'ninja': 
    print (bcolors.FAIL + "---> !!! Not found parameters of build use '-b' + 'ninja|vs32|vs64  !!!" + bcolors.ENDC)
    sys.exit(1)
  sys.exit(main(namespace))