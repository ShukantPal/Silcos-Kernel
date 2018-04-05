### What is restrictive linking?

The kernel is a vast network of components stiched together at runtime to execute parallely, when possible. Software may load or de-load a kernel module at runtime which may inturn lead to symbol conflicts. For example, a IPC module declares a `DataChannel` class, while the UDI environment also internally uses a `DataChannel` class. This may cause confusion in the `ModuleLoader` over where to store these symbols.

To solve this problem, the concept of restrictive linking is introduced. The IPC module is actually supposed to be loaded by a `AppContainer` in user-space. Therefore, it doesn't need to even link with the UDI environment and it shouldn't, for security reasons. By not linking with any driver, in-built module, you can ensure that it uses the standard APIs that the `KMAppFramework` 
provides (SDK for kernel-modules used for connecting processes across address spaces). This is implemented by keeping it under a Namespace - `::KMAppFramework` and giving it the global identifier `::KMAppFramework::AppName::IPC`. That means it will be able to link only with the `KMAppFramework`.

# Namespace

	class Namespace : protected LinkedListNode
	{
	public:
		const char *fileName;
		const char *nsLocalIdentifier;
		const char *nsGlobalIdentifier;
	
		Namespace *create(const char *name)
		{
			if(searchOnly(name, strlen(name)) == null)
			{
				Namespace *newChild = new Namespace(name, this);
				AddElement(newChild, &nsChildSet);
				return (newChild);
			}
			else
			{
				return (null);
			}
		}
	
		Namespace *create(const char *fromFile, const char *name)
		{
			if(searchOnly(name, strlen(name)) == null)
			{
				Namespace *newChild = new Namespace(fromFile, name,
						this);
				AddElement(newChild, &nsChildSet);
				return (newChild);
			}
			else
			{
				return (null);
			}
		}
	
		Namespace *searchOnly(const char *dChildIdentBuffer, unsigned bytes);
		Namespace *searchRelative(const char *dChildIdentBuffer);
	
		static Namespace *search(const char *globalIdentifier, bool object);
		static Namespace *root(){ return (const_cast<Namespace*>(&nsRoot)); }
	protected:
		Namespace *nsOwner;
		LinkedList nsChildSet;
		unsigned long totalChildren;
		Namespace(const char *name, Namespace *owner);
		Namespace(const char *fileName, const char *name, Namespace *owner);
	
		void addToParent(){ AddElement(this, &nsOwner->nsChildSet); }
	private:
		static const Namespace nsRoot;
		Namespace();
	};

This is the base class for restrictive linking and other features that help resolve kernel components into a organized hierarchy of objects. At its core, a `Namespace` is just able to be child of another `Namespace` and hold a list of direct child `Namespace`s which forms a topological tree. It also provides a few methods to search child namespaces by their path & identifier. In itself, a `Namespace` holds no functionality.

### Namespace::Namespace
This is the default constructor for `Namespace` and is a private member. It is only used for the `nsRoot` object which is the indirect parent of all other namespaces.

### Namespace::fileName
(optional field)

This is a optional field which may contain the object-file name associated with this namespace (e.g. name of the kernel-module file loaded).

### Namespace::nsLocalIdentifier
The local identifier is the direct name of the `Namespace` in concern. For example, the local identifier of the UDI driver namespace is `UDI`. This must not contain any `::` or `:` token as they separate local identifiers of parent-child namespaces.

### Namespace::nsGlobalIdentifier
The global identifier is the full path to the `Namespace` in concern. For example, the local identifier of the UDI ethernet driver would be `::UDI::Network::Ethernet`. It could be used to search it through the whole namespace tree without any initial parent namespace (as the `nsRoot` namespace is the topmost indirect parent).

### Namespace::create()

**Prototype:** `Namespace *Namespace::create(const char *name`

**Description:**

This creates a direct child namespace with the given local identifier. The global identifier of the namespace is automatically created as the new `Namespace` object will be the direct child of this namespace.

**Parameters:**

`const char *name` - local-identifier of this namespace

### Namespace::create()

**Prototype:** `Namespace *Namespace::create(const char *fileName, const char *name)`

**Description:**

This creates a direct child with the given local identifier and associated file-name given.

**Parameters:**

`const char *fileName` - name of the associated object-file/any-other-file for this namespace

`const char *name` - local identifier for this `Namespace`

### Namespace::searchOnly()

**Prototype:** `Namespace *Namespace::searchOnly(const char *dChildIdentBuffer, unsigned bytes)`

**Description:**

This is used for searching a direct-child of this `Namespace` object which has the given local identifier, with the no. of bytes given. This identifier is stored in an buffer which may or may not end with a null-terminator, and hence, the no. of characters must be passed. One use of this method is in `Namespace::searchRelative` which recursively uses this method to find the next namespace in path, where a portion of the global identifier given is passed to this method. E.g. `::UDI::Network::Ethernet` is passed to `searchRelative()` and the `Network` namespace calls `searchOnly("Ethernet", strlen("Ethernet"))` to this method in the global identifier itself.

**Parameters:**

`const char *dChildIdentBuffer` - This stores the local identifier of the required namespace and its size is also passed.

`unsigned bytes` - The size of the `dChildIdentBuffer` string holding the local identifier.

### Namespace::searchRelative()

**Prototype:** `Namespace *Namespace::searchRelative(const char *dChildIdentBuffer)`

**Description:**

Searches for a indirect/direct child namespace of this, such that the string passed holds its relative path to this namespace. For example, the `UDI` namespace object may be used to find the `Ethernet` namespace using `udi_namespace_object->searchRelative("Network::Ethernet")`.

**Parameters:**

`const char *dChildIdentBuffer` - buffer holding the relative identifier for the namespace required. This must be null-terminated.

### Namespace::search()

**Prototype:** `static Namespace::search(const char *globalIdentifier, bool object)`

**Description:**

Searches for the namespace with the given global identifier. The search starts from `nsRoot` or `Namespace::root()` as it is the topmost node of the namespace tree.

**Parameters:**

`const char *globalIdentifier` - Global identifier of the namespace required, which may be accessed using `required_namespace_object->nsGlobalIdentifier`.

`bool object` - reserved for future; may be used to extend the namespace tree with objects that have dynamic functionality.

### How is namespace used to link symbols?

The `Namespace` object cannot be directly used for restrictive linking. It is inherited by the `ModuleContainer` class which holds all the information related to a specific kernel-module. These `ModuleContainers` further hold `SymbolLookup` table pointers grouping together related modules with a pool of symbols. This has not been documented yet! We'll talk about it later. A new class `Jumper` class may be introduced later to pass on these symbol tables to grandchildren and more in the future while building the UDI framework.