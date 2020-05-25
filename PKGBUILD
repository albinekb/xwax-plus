# Maintainer: Peter Vasil <mail@petervasil.net>
# Contributor: Lukas Fleischer <lfleischer@archlinux.org>
# Contributor: sputnick <gilles DOT quenot AT gmail DOT com>

pkgname=exwax-git
_name=exwax
pkgver=1.6.beta2.31.g3f97662
pkgrel=1
pkgdesc='Open-source vinyl emulation software for Linux. Extended Version including interactive BPM display, Album and Genre Columns, Cover Art Display, Dicer Navigation, Cue display and saving'
arch=('i686' 'x86_64')
url='http://www.xwax.co.uk/'
license=('GPL')
depends=('alsa-lib' 'sdl_ttf' 'sdl_image' 'ttf-dejavu' 'jack')
optdepends=('cdparanoia: for CD import'
            'mpg123: for MP3 import'
            'ffmpeg: for video fallback import')
source=("${pkgname}::git+https://gitlab.com/K3nn3th/exwax.git")
md5sums=('SKIP')
conflicts=('exwax')
provides=('exwax')

pkgver() {
  cd "${srcdir}/${pkgname}"
  git describe | sed 's/^v//;s/-/./g;s/_/./g;'
}

build() {
  cd "${srcdir}/${pkgname}"

  ./configure --enable-alsa --enable-jack --prefix /usr
  make EXECDIR="/usr/share/${_name}"
}

package() {
  cd "${srcdir}/${pkgname}"
  make DESTDIR="${pkgdir}" EXECDIR="/usr/share/${_name}" install
}
