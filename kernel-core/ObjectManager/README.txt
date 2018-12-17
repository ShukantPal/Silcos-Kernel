KTerm is a external subsystem management engine for the kernel. It is helpful in debugging subsystem status after a kernel failure and also getting information about the kernel at runtime. It
allows the kernel to communicate with user-mode processes without requiring any special communication method. After a subsystem registers its infomation with KTerm, the information is securely
delivered at the system-level along with security parameters.

KTerm defines the architecture of the running system and can be given a simple GUI for communicating with the user. Simple operating systems based on the CircuitExecutive project can implement a
GUI consisting of a terminal-interface which hotplugs with a shell that can communicate the CircuitExecutive::KTerm.

Syntax of KTerm interface:

Commands (for client/user/user-mode process/other)

RCSD - Read Current Subsystem Directory 
WCSD - Change the current subsystem-directory 

Example -
After starting a KTerm session- 

WCSD KTerm::Core::KFrameManager - this will get you to the PMA or kframe-manager subsystem of the kernel-core (microkernel)

RCSD - will give KTerm::Core::KFrameManager

--

Package Naming

RCPN - Read Current Package Name

Example - will given silcos.core.kframe-manager for subsys-dir KTerm::Core::KFrameManager

WRITE .::CACHE_LIMIT
READ .::CACHE_COUNT

-----------------------------------------

Registering a subsys -

KtRegisterServer(...)
KtActivateServer(KT_SERVER_HANDLE)
KtDeactivateServer(KT_SERVER_HANDLE)
KtExitServer(KT_SERVER_HANDLE, KT_SECURITY_KEY)

-----------------------------------------

Internal funcs -

KModuleMain
KtCreateSession()
KtWriteSession()
KtReadSession()
KtDestroySession()

KtPlugClient()
KtDeplugClient()

VerfiyServerClientConnect()