import os.path
from distutils.core import setup
from distutils.command.build import build
import os
import platform
import sys
import re

import subprocess
from distutils.version import LooseVersion


class BuildMITIE(build):
    def run(self):
        if platform.system() == "Windows":
            if LooseVersion(self.get_cmake_version()) < '3.1.0':
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

            if not os.path.exists('mitielib/build'):
                os.makedirs('mitielib/build')
            os.chdir('mitielib/build')
            if sys.maxsize > 2**32:
                subprocess.check_call(['cmake', '..', '-A', 'x64'])
            else:
                subprocess.check_call(['cmake', '..'])

            subprocess.check_call(['cmake', '--build', '.', '--config', 'Release', '--target', 'install'])
            os.chdir('../..')

            build.run(self)
        
            self.copy_file(
                'mitielib/mitie.dll',
                os.path.join(self.build_lib, 'mitie/mitie.dll'))
        else:
            subprocess.check_call(['make', 'mitielib'])
            build.run(self)
            self.copy_file(
                'mitielib/libmitie.so',
                os.path.join(self.build_lib, 'mitie/libmitie.so'))

    def get_cmake_version(self):
        try:
            out = subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError("CMake must be installed to build the following extensions: " +
                               ", ".join(e.name for e in self.extensions))
        return re.search(r'version\s*([\d.]+)', out.decode()).group(1)


setup(
    version='0.7.0',
    name='mitie',
    packages=['mitie'],
    package_dir={'mitie': 'mitielib'},
    cmdclass={'build': BuildMITIE},
    classifiers=[
        'Operating System :: MacOS :: MacOS X',
        'Operating System :: POSIX',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'License :: Boost License',
        ]
    )
