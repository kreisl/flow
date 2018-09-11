# Building flow

## Prerequisites

* ROOT 6 \(tested with the versions 6.15 and 6.14\)
* gcc or clang with C++14 support
* cmake 2.8

{% hint style="info" %}
For Building on GSI kronos please see [below](building-flow.md#building-on-kronos).
{% endhint %}

##  Building

To build please read the follow steps:

#### Load ROOT6 in the terminal

```bash
$ source /path/to/root/bin/thisroot.sh
```

#### Configure with cmake

```bash
$ cmake ../path/to/source/
```

{% hint style="info" %}
To configure the install path the usage of the cmake option

**CMAKE\_INSTALL\_PREFIX** is recommended.

`$ cmake -DCMAKE_INSTALL_PREFIX=/path/to/install/ /path/to/source`
{% endhint %}

#### Build with make

```bash
$ make -jN
```

 where N is the number of parallel jobs.

#### Install with make

```bash
$ make install -jN
```

### Building on kronos

{% hint style="danger" %}
**Prerequisites on kronos**  
please be advised that the default gcc on kronos is not sufficient to build the library. A newer version is available on cvmfs.
{% endhint %}

#### Use the modules version on /cvmfs/it.gsi.de/

A guide from the IT is available in the following directory

```bash
/cvmfs/it.gsi.de/modules/HowTo
```

For convenience I replicated parts of the HowTo here:

To automatically add the module section to the shell add the following lines to your `.bashrc`

```bash
if [[ -z $MODULESHOME && -f /etc/profile.d/modules.sh ]]; then
 source /etc/profile.d/modules.sh
fi
alias module='module -u exp'
unset MODULEPATH
if [[ $MODULESHOME && -d /cvmfs/it.gsi.de/modules ]]; then
 module use /cvmfs/it.gsi.de/modules
fi
```

To load the new gcc version one can call:

`$module load gcc/6.4.0`

In order to make `cmake` choose gcc it may be necessary to use the following lines

```bash
export CC=gcc
export CXX=g++
```

It is now possible to follow the guide [above](building-flow.md#building).

