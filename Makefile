# S i l c o s   K e r n e l   B u i l d   F i l e
#        (B u i l d  &  T e s t  -  M a i n)
#
# C o m m a n d s :
#
# q, b, Build, CleanBuild
#

Build:
	make BuildChain -C ./KernelHost
	make IA32_HAL   -C ./HAL
	make MfMake -C ./ModuleFramework
	make ObMake -C ./ObjectManager
	make RsMake -C ./ResourceManager
	cp KernelHost/Build/circuitk-1.02 ../circuit-iso/boot
	cp Modules/Builtin/* ../circuit-iso/boot

q: Build
	grub-mkrescue -o os.iso ../circuit-iso --modules="iso9660 part_msdos multiboot"
	qemu-system-i386 -cdrom os.iso -boot d -m 512 -smp cpus=2,cores=1,sockets=1 -display sdl

b: Build
	grub-mkrescue -o os.iso ../circuit-iso --modules="iso9660 part_msdos multiboot"
	bochs