import time
import argparse
import subprocess
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

    def check_depends(self):
        print(Installer.info, 'Установка зависимостей...', Installer.reset)
        if 'MAJARO' or 'ARCH' in Installer.release:
            os.system('pacman -S --needed cmake gcc ninja openssl')
        elif 'Ubuntu' in Installer.release:
            os.system('apt-get install build-essential autoconf libtool pkg-config libgflags-dev\
            libgtest-dev clang libc++-dev -y')
        else:
            print(Installer.critical, 'ERROR! Platform not supported!', Installer.reset)

    def building(self, args):
        os.mkdir(r"build")
        os.chdir('./build')
        os.chdir('../')
        os.chdir('./build')
        if args[0] == 'ninja':
            os.system('cmake .. -GNinja')
            os.system(f'cmake --build . --config Release --parallel 16')
            if args[0] == 'win32':
                os.system('cmake .. -G "Visual Studio 15 2017"')
            if args[0] == 'win64':
                os.system('cmake .. -G "Visual Studio 16 2019"')

    def parse_args(self):
        parser = argparse.ArgumentParser(usage='sudo python3 install.py -b ninja')
        parser.add_argument('-b', '--build', nargs=2, required=True, help='Build FHT', dest='b')
        return parser.parse_args()


if __name__ == '__main__':
    install = Installer()
    args = install.parse_args()
    install.check_depends()
    install.building(args.b)




