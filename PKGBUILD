# macfand PKGBUILD
#
# Maintainer: Aaron Blakely <aaron@ephasic.org>
pkgname=macfand-git
pkgver=1.0.0.r24.c6a99d1
pkgrel=1
epoch=
pkgdesc="Mac fan control daemon"
arch=(x86_64 i686)
url="https://github.com/ablakely/macfand.git"
license=('MIT')
groups=()
depends=()
makedepends=(git)
checkdepends=()
optdepends=()
provides=(macfand)
conflicts=(macfand)
replaces=()
backup=()
options=()
install=
changelog=
source=("git+$url")
noextract=()
md5sums=('SKIP')
validpgpkeys=()

pkgver() {
        cd "${_pkgname}"
        printf "1.0.0.r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
	cd macfand
	make
}

package() {
	cd macfand
	make install
	install -Dm644 LICENSE "${pkgdir}/usr/share/licenses/${pkgname}/LICENSE"
	install -Dm644 README.md "${pkgdir}/usr/share/doc/${pkgname}/README.md"
	install -Dm644 macfand.1 "${pkgdir}/usr/share/man/man1/{$pkgname}/macfand.1"
}
