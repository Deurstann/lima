# LIMA - Libre Multilingual Analyzer

LIMA is mutliplatform. It has been developed under GNU/Linux and recently 
compiled and run under MS Windows. Its installation procedure under Linux
is described bellow. Installation instructions under Windows are still to be 
written.

The various modules (lima_*) have different purposes:

 * lima_common
    contains tools shared by all modules (eg. factories)
 * lima_linguisticprocessing
    LIMA linguistic analyser
 * lima_linguisticdata
    linguistic ressources used in LIMA

## Install

Build dependencies: cmake, c++ (tested with gcc), gawk, NLTK, libraries and 
development packages for : boost , Qt5, Qwt, enchant (optional, for  
orthographic correction), qhttpserver (optional, for lima http/json API).

Under Ubuntu 14.04, most of these dependencies are installed with the following packages:
```
$ sudo apt-get install python-nltk gawk cmake ninja-build qt5-default libqt5xmlpatterns5 \
libqt5xmlpatterns5-dev qttools5-dev build-essential libboost-all-dev libenchant-dev \
mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev libasan0 qml-module-qt-labs-folderlistmodel \
qml-module-qt-labs-settings qtdeclarative5-dev
```
Under Mageia, most of these dependencies are installed with the following 
packages:
$ sudo urpmi cmake, lib64qwt-devel, libasan1 (to be completed)

qhttpserver can be downloaded and installed from 
https://github.com/aymara/qhttpserver

As we were not able to find a Free part of speech tagged English corpus, LIMA 
depends for analyzing English on freely available but not free data that you 
will have to download and prepare yourself. This data is an extract of the Peen 
treebank corpus available for fair use in the NLTK data. To install, please 
refer to http://nltk.org/data.html. Under Ubuntu this can be  done like that:
```bash
$ sudo apt-get  install python-nltk
$ python
>>> import nltk
>>> nltk.download()
d dependency_treebank
```

:warning: **Note:** Under Ubuntu 14.04, you can get an HTTP 404 error while downloading nltk data. Please refer to [this StackExchange question](http://askubuntu.com/questions/527388/python-nltk-on-ubuntu-12-04-lts-nltk-downloadbrown-results-in-html-error-40) for a solution.


Then prepare the data for use with LIMA by running the following commands:

```bash
$ cd $HOME/nltk_data/corpora/dependency_treebank
$ cat wsj_*.dp | grep -v "^$" > nltk-ptb.dp
```

:warning: **If you havn't already downloaded LIMA git repository (source code), please [do it now](https://github.com/aymara/lima.git).**

Move to the root of the LIMA  git repository, e.g.:
```bash
$ cd $HOME/lima
```

You need to set up a few environment variables. For this purpose, you can 
source the setenv-lima.sh script from the root of **the LIMA git repository** (please check 
values before):
```bash
$ source ./setenv-lima.sh -m release
```

Finally, from the LIMA repository root, run:
```bash
$ ./gbuild.sh -m release
```

This builds LIMA in release mode, assuring the best performance. To report bugs for example, you should build LIMA in debug mode. To do so, just omit the `-m release` option when invoking `setenv-lima.sh` and `gbuild.sh`. 

Alternatively, you can 

1. define the following environment variables manually:

`LIMA_DIST`             binaries and libraries
`LIMA_EXTERNALS`        dependencies
`LIMA_RESOURCES`        any kind of ressources (including training data)
`LIMA_CONF`             configuration folder
`LINGUISTIC_DATA_ROOT`  path to the lima_linguisticdata project root
`NLTK_PTB_DP_FILE`      path to the Penn treebank extract from NLTK (see below)

2. set `PATH` and `LD_LIBRARY_PATH`:

```
export PATH=$LIMA_DIST/bin:$LIMA_EXTERNALS/bin:$PATH
export LD_LIBRARY_PATH=$LIMA_EXTERNALS/lib:$LIMA_DIST/lib
```

3. run `gbuild.sh` 

## Build troubleshoutings

* If you use your own compiled boost libraries alongside system boost libraries 
AND cmake fails on lima_linguisticprocessings indicating it found your boost 
version headers but it uses the system libraries, add the following definition 
at the beginning of the root CMakeLists.txt of each subproject : 
set(Boost_NO_SYSTEM_PATHS ON)
* If some packages are not found at configure time (when running cmake), double check the dependencies packages you have installed. If it's OK, maybe we missed to indicate a dependency. Then, don't hesitate to open an issue. Or submit a merge request that solves the problem.

