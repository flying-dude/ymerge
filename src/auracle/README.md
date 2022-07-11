Using the C++ code from the discontinued [Auracle](https://github.com/falconindy/auracle) project.
User [sandsmark](https://github.com/sandsmark/auracle) has ported the original auracle project to the cmake build system.

See Reddit announcement [here](https://www.reddit.com/r/archlinux/comments/vtoomp/picking_up_development_of_the_discontinued/).

## What is Auracle?

Auracle is a command line tool used to interact with Arch Linux's User
Repository, commonly referred to as the [AUR](https://aur.archlinux.org).

### Features

Auracle has a number of actions it can perform:

* `search`: find packages in the AUR by regular expression.
* `info`: return detailed information about packages.
* `show`: show the contents of a source file for a package (e.g. the PKGBUILD)
* `raw{info,search}`: similar to info and search, but output raw json responses
  rather than formatting them.
* `clone`: clone the git repository for packages.
* `buildorder`: show the order and origin of packages that need to be built for
  a given set of AUR packages.
* `outdated`: attempt to find updates for installed AUR packages.
* `update`: clone out of date foreign packages

### Non-goals

Auracle does not currently, and will probably never:

* Build packages for you.
* Look at upstream VCS repos to find updates.

### Building and Testing

Building auracle requires:

* A C++17 compiler
* cmake
* ninja
* libsystemd
* libalpm
* libcurl

Testing additionally depends on:

* gmock
* gtest
* python3

You're probably building this from the AUR, though, so just go use the
[PKGBUILD](https://aur.archlinux.org/packages/auracle-git).

If you're hacking on auracle, you can do this manually:

```sh
$ cmake -GNinja .
$ ninja
```

And running the tests is simply a matter of:

```sh
$ ninja test
```

### Usage

```
$ auracle --help
auracle [options] command

Query the AUR or clone packages.

  -h, --help               Show this help
      --version            Show software version

  -q, --quiet              Output less, when possible
  -r, --recurse            Recurse dependencies when cloning
      --literal            Disallow regex in searches
      --searchby=BY        Change search-by dimension
      --color=WHEN         One of 'auto', 'never', or 'always'
      --sort=KEY           Sort results in ascending order by KEY
      --rsort=KEY          Sort results in descending order by KEY
      --resolve-deps=DEPS  Include/exclude dependency types in recursive operations
      --show-file=FILE     File to dump with 'show' command
  -C DIR, --chdir=DIR      Change directory to DIR before cloning
  -F FMT, --format=FMT     Specify custom output for search and info

Commands:
  buildorder               Show build order
  clone                    Clone or update git repos for packages
  info                     Show detailed information
  outdated                 Check for updates for foreign packages
  rawinfo                  Dump unformatted JSON for info query
  rawsearch                Dump unformatted JSON for search query
  search                   Search for packages
  show                     Dump package source file
  update                   Clone out of date foreign packages
  ``` 