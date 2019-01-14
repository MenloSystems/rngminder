#!/bin/bash -e
#
#   Usage: update.sh
#
# Downloads fresh copies of the various other files in this directory,
# overwriting any old versions.
#
# The versions to use can be set with environment variables, which default
# to using the newest (development) version available.
#
# Copyright (C) 2016,2017  Olaf Mandel <olaf@mandel.name>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program.  If not, see
# <http://www.gnu.org/licenses/>.
#

# Define version defaults
MATHJAX_VER=${MATHJAX_VER:-HEAD}
AC_ARCHIVE_VER=${AC_ARCHIVE_VER:-HEAD}


### No configurable parts below ###

function get_file() {
    local url=$1
    shift
    wget -nv -O- "${url}"
}

function get_github() {
    local repo=$1
    local ver=$2
    local file=$3
    shift 3
    get_file "https://raw.githubusercontent.com/${repo}/${ver}/${file}"
}

# Main routine
d=$(readlink -f "$(dirname "$0")")

mkdir -p "${d}/js"
(get_github mathjax/MathJax "${MATHJAX_VER}" MathJax.js &&\
 get_github mathjax/MathJax "${MATHJAX_VER}" extensions/MathEvents.js &&\
 get_github mathjax/MathJax "${MATHJAX_VER}" extensions/MathMenu.js &&\
 get_github mathjax/MathJax "${MATHJAX_VER}" extensions/MathZoom.js &&\
 get_github mathjax/MathJax "${MATHJAX_VER}" jax/element/mml/jax.js &&\
 get_github mathjax/MathJax "${MATHJAX_VER}" jax/input/TeX/config.js &&\
 get_github mathjax/MathJax "${MATHJAX_VER}" jax/input/TeX/jax.js &&\
 get_github mathjax/MathJax "${MATHJAX_VER}"\
                                            jax/output/NativeMML/config.js &&\
 get_github mathjax/MathJax "${MATHJAX_VER}" jax/output/NativeMML/jax.js\
) >"${d}/js/MathJax.js"

mkdir -p "${d}/license"
get_file "https://www.apache.org/licenses/LICENSE-2.0.txt" >"${d}/license/ALv2"

mkdir -p "${d}/m4"
for i in ax_ac_append_to_file ax_ac_print_to_file ax_add_am_macro_static\
 ax_am_macros_static ax_check_gnu_make ax_code_coverage ax_file_escapes\
 ax_gnu_autotest ax_prog_doxygen ax_valgrind_check;
do
    get_github peti/autoconf-archive "${AC_ARCHIVE_VER}" m4/${i}.m4\
     >"${d}/m4/${i}.m4"
done
