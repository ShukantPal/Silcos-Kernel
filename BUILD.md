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