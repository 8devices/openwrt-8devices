https://www.8devices.com/wiki/mango:build

To build firmware you need Ubuntu 16.04 LTS, other distributions may work but are not supported. For example, Ubuntu 20.04 is known to not work. Building firmware directly on a Windows platform is not supported. However, if all you have available is a Windows machine then we would recommend to set up a Linux based virtual machine.

Note: the following build was made on Ubuntu 16.04.7 LTS running 4.4 kernel.

Note: please build with non-root privileges!

Once you have a Linux OS running install the following prerequisite packages:

```
sudo apt-get update
sudo apt-get install git git-doc subversion build-essential flex wget gawk unzip man file python2.7 zlib1g-dev libssl-dev libncurses5-dev ocaml-nox
```

Get latest sources from 8devices OpenWrt branch:

```
git clone --branch qsdk-11.3-ipq60xx-4.4 https://github.com/8devices/openwrt-8devices.git
cd openwrt-8devices
```

The easiest way is simply to execute a build script and wait until firmware will be built.

```
./quick_start.sh 8dev_mango_open  -c
```

Building is a CPU heavy process and may take a long time depending on your CPU capabilities.

Once finished the compiled firmware image ending with openwrt-ipq-ipq60xx-8devices-mango-dvk-squashfs-sysupgrade.bin will be placed under bin/ipq/ directory.
