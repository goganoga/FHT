import time
import argparse
import platform
import os
import colorama


class Installer:
    info = colorama.Fore.BLUE
    ok = colorama.Fore.GREEN
    warning = colorama.Fore.LIGHTRED_EX
    critical = colorama.Fore.RED
    reset = colorama.Style.RESET_ALL
    system = platform.system()
    release = platform.release()
    vs32 = '"Visual Studio 15 2017"'
    vs64 = '"Visual Studio 16 2019"'
    ninja = 'Ninja'

    def check_depends(self):
        print(Installer.info, 'Install Dependent Libraries', Installer.reset)
        time.sleep(3)
        if 'MAJARO' or 'ARCH' in Installer.release:
            os.system('pacman -S --needed cmake gcc ninja openssl')
        elif 'Ubuntu' in Installer.release:
            os.system('apt-get install build-essential autoconf libtool pkg-config libgflags-dev\
            libgtest-dev clang libc++-dev -y')
        elif 'Windows' in Installer.system:
            os.environ['CC']='cl.exe'
            os.environ['CXX']='cl.exe'
        else:
            print(Installer.critical, 'ERROR! Platform not supported!', Installer.reset)

    def building(self, args):
        if os.path.isdir(r"build") != True:
            os.mkdir(r"build")
            os.chdir('./build')
            os.chdir('../')
        os.chdir('./build')
        if args.build[0] == 'ninja':
            os.system(f'cmake .. -G {Installer.ninja}')
            os.system(f'cmake --build . --config Release --parallel {args.parallel[0]}')
            if args[0] == 'vs32':
                os.system(f'cmake .. -G {Installer.vs32}')
            if args[0] == 'vs64':
                os.system(f'cmake .. -G {Installer.vs64}')

    def parse_args(self):
        parser = argparse.ArgumentParser(usage='sudo python3 install.py -b ninja|vs32|vs64 -p 16')
        parser.add_argument('-b', '--build', nargs=1, required=True, choices=['ninja', 'vs32', 'vs64'])
        parser.add_argument('-p', '--parallel', nargs=1, required=True, choices=['16'])
        return parser.parse_args()


if __name__ == '__main__':
    install = Installer()
    args = install.parse_args()
    install.check_depends()
    install.building(args)




