# How it Works

The `ymerge` tool operates on a set of curated packages maintained in a git
[repository](https://github.com/flying-dude/curated-aur).
The command `ymerge --sync` will download this repository or update to the latest changes, if already downloaded.

The curated-aur repository contains PKGBUILD package recipes as well as a set of
[whitelisted](https://github.com/flying-dude/curated-aur/blob/main/aur-whitelist.json)
packages from the Arch Linux AUR.
These packages have been reviewed and approved by maintainership of curated repo.

Since review has already been conducted, `ymerge` does not require the user to manually review the whitelisted packages. Installation is simple and intuitive in these cases. Manual review for non-whitelisted packages is a planned feature.

Existing AUR helpers like `yay` and `paru` are whitelisted and can be installed with `ymerge`. In addition, `ymerge` ships the previously discontinued
[`auracle`](https://github.com/flying-dude/ymerge/blob/main/src/auracle/README.md)
AUR helper.
