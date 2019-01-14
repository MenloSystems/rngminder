RNG entropy-pool minder
=======================

Stores the entropy-pool on system shutdown and restores it on next boot.
Compared to using plain writes to `/dev/(u)random`, this executable has
the advantage of actually incrementing the entropy-count.

Compile and install the usual autotools-way:

    ./autogen  # if taken from a repository
    ./configure
    make
    make install

License
-------

rngminder: RNG entropy-pool minder  
Copyright (C) 2019 MenloSystems GmbH

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

The above license covers all files of the program except those listed
below. Those exceptions are covered by the licenses stated inside the
files. Where directories are listed, this means all files below these
directories.

* `aclocal.m4`
* `build-aux/`
* `configure`
* `external/`
* `Makefile.in`

The license texts are available in the `external/license` folder for
convenience.
