# package recipe for the development version of ymerge
# note: copy this into an empty folder before running makepkg. otherwise this will modify your git repo.

pkgname=ymerge
pkgver=0.0.2.alpha
pkgrel=1
pkgdesc="A Source-Based Package Manager for Arch Linux."
arch=(x86_64)
url="https://github.com/flying-dude/ymerge/"
license=(BSD)
depends=(fmt git abseil-cpp)
makedepends=(cmake fmt nlohmann-json ninja gtest)
provides=(ymerge)

source=(
    "git+https://github.com/flying-dude/ymerge.git" # to work locally, replace with: git+file:///home/$user/.../ymerge
    "https://github.com/Jorengarenar/libXDGdirs/archive/84ab4117b787c4079965c98b4578a38b63795cc0.zip"
)

b2sums=('SKIP'
        '816f464b468ce668cb3bf4e869e683c045a8083ec43f020df2096189a408890e41b3ec7a26b72f06cf16ed31883228f810385968162baa5c87b61cdbe37004ff')

pkgver() {
    cd ymerge
    printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
}

build() {
    cp --no-target-directory --recursive libXDGdirs-* ymerge/src/submodules/libxdgdirs/
    cd ymerge

    cmake -S . -B build -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_SKIP_RPATH=On \
        -DYMERGE_VERSION="${pkgver}" \
        -DCMAKE_INSTALL_PREFIX:PATH='/usr'

    cmake --build build
}

package() {
    cd ymerge/build/
    DESTDIR="${pkgdir}" ninja install
}
