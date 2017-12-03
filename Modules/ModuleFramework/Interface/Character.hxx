#ifndef MODULES_MODULEFRAMEWORK_INTERFACE_CHARACTER_HXX_
#define MODULES_MODULEFRAMEWORK_INTERFACE_CHARACTER_HXX_

class Character
{
public:

	/*
	 * Check whether the given character is a lower-case letter & returns true
	 * if so. For all non-letter characters, false is returned.
	 */
	static inline bool isLowerCase(char ch)
	{
		if(ch >= 'a' && ch <= 'z')
			return (true);
		else
			return (false);
	}

	/*
	 * Check whether the given character is a upper-case letter & returns true
	 * if so. For all non-letter characters, false is returned.
	 */
	static inline bool isUpperCase(char ch)
	{
		if(ch >= 'A' && ch <= 'Z')
			return (true);
		else
			return (false);
	}

	/*
	 * Returns lower-case version of a letter (which is upper-case) or if the
	 * given character is not a upper-case letter, 0 is returned.
	 */
	static inline char toLowerCase(char ch)
	{
		if(ch >= 'a' && ch <= 'z')
			return (ch + 32);
		else
			return (0);
	}

	/*
	 * Returns upper-case version of a letter (which is lower-case) or if the
	 * given character is not a lower-case letter, 0 is returned.
	 */
	static inline char toUpperCase(char ch)
	{
		if(ch >= 'A' && ch <= 'Z')
			return (ch - 32);
		else
			return (0);
	}

private:
	Character();
};

#endif/* ModuleFramework/Character.hxx */
