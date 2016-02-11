import os.path
from distutils.core import setup
from distutils.command.build import build

import subprocess


class BuildMITIE(build):
    def run(self):
        subprocess.check_call(['make', 'mitielib'])
        build.run(self)
        self.copy_file(
            'mitielib/libmitie.so',
             os.path.join(self.build_lib, 'mitie/libmitie.so'))


setup(
    version='0.2.0',
    name='mitie',
    packages=['mitie'],
    package_dir={'mitie': 'mitielib'},
    cmdclass={'build': BuildMITIE},
    )
