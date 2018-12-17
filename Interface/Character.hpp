#ifndef MDFRWK_CHARACTER_HXX_
#define MDFRWK_CHARACTER_HXX_

class Character
{
public:
	static inline bool isDigit(char ch)
	{
		return (ch >= '0' && ch <= '9');
	}

	static inline bool isLowerCase(char ch)
	{
		if(ch >= 'a' && ch <= 'z')
			return (true);
		else
			return (false);
	}

	static inline bool isUpperCase(char ch)
	{
		if(ch >= 'A' && ch <= 'Z')
			return (true);
		else
			return (false);
	}

	static inline char toLowerCase(char ch)
	{
		if(ch >= 'a' && ch <= 'z')
			return (ch + 32);
		else
			return (0);
	}

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
