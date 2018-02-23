# Build Environment

## Required Software to Build the Cross-Compiler

To build the Silcos kernel on a linux host, which is the only supported currently, you must first install the some basic utility programs. These will help you create the cross-compiler for the Silcos system -

1. GCC
2. G++ (if building a version of GCC >= 4.8.0)
3. GNU Make
4. GNU Bison
5. Flex
6. GNU CMP
7. GNU MPFR
8. GNU MPC
9. Texinfo
10. ISL (optional)
11. CLooG (optional)

### For APT-based Systems

    sudo apt install libgmp3-dev
    sudo apt install libmpfr-dev
    sudo apt install libisl-dev
    sudo apt-install libcloog-isl-dev
    sudo apt-install libmpc-dev
    sudo apt-install texinfo

Now you are prepared for building the cross-compiler.

## Cross-Compiler

Let us export some environment variables to make our life easier. We will install the cross-compiler into the `~/opt/cross` directory. You may also install it in the `/usr/local/cross/` directory to allow others to use the cross-compiler by setting the PREFIX variable. Your target can be i386-elf or i686-elf (not supported yet).

	export PREFIX="$HOME/opt/cross"
	export TARGET=i386-elf
	export PATH="$PREFIX/bin:$PATH"

We need to download the source-code of `binutils` and `gcc`.  You can do so from `https://www.gnu.org/software/binutils/` and `https://www.gnu.org/software/gcc/`. Here we will store them at `~/Silcos-CrossCompile`.

	echo 'We assume that the GCC & Binutils source is downloaded at ~/Silcos-CrossCompile`
	cd $HOME/Silcos-CrossCompile
	mkdir build-binutils
	../binutils-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
	make
	make install
	cd ..

	which -- $TARGET-as || echo $TARGET-as is not in the PATH (this part should not come, otherwise some error already occured, note)
	mkdir build-gcc
	cd build-gcc
	../gcc-x.y.z/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
	make all-gcc
	make all-target-libgcc
	make install-gcc
	make install-target-libgcc

See the OSDev Wiki (GCC_Cross-Compiler) page for better, wider, clearer explaination.


# Building the silcos kernel

To create a developer environment is very easy for this kernel. All you need to do is download this source code (in say folder "X"). Now in folder "X" as current directory -

    mkdir boot
    cd boot
    mkdir grub
    cd grub

Now, in /path/to/X/boot/grub create a file grub.cfg with the following contents -

    set timeout=15
	set default=0
    
	menuentry "Silcos"
	{
		multiboot2 /boot/circuitk-1.02
		module2 /boot/kernel.silcos.hal kernel.silcos.hal
		module2 /boot/kernel.silcos.excmgr kernel.silcos.excmgr
		module2 /boot/silcos.obmgr
		module2 /boot/silcos.mdfrwk
		module2 /boot/silcos.rsmgr
		boot
	}

In the terminal,

    echo we have named this folder circuit-source-2.03! Could be anything you'd like!
    cd /path/to/X/circuit-source-2.03
    sudo apt-get install qemu
    make q