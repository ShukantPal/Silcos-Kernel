/**
 * File: String.hxx
 *
 * Summary:
 * Declares the immutable String class, which is used for representing character
 * sequences in OOP manner & allowing sharing of their memory by the generic
 * StringPool pooling mechanism (@See "StringPool.hxx")
 * 
 * Classes:
 * String - Represents a immutable sequence of characeters
 *
 * Origin:
 * Provides a portable interface for managing character sequences in the OOP
 * manner.
 *  ___________________________________________________________________
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 * Copyright (C) 2017 - Shukant Pal
 */
#ifndef __MDFRWK_STRING_HXX__
#define __MDFRWK_STRING_HXX__

#include <Memory/KObjectManager.h>
#include <Object.hxx>
#include <TYPE.h>

#define MAX_STRING_LENGTH 480 // Maximum length of string, & its source character buffer

enum StringAttributes
{
	ReadOnly = (1 << 0),
	ConstSize = (1 << 1)
};

extern ObjectInfo *tString;
extern char nmString[];

class StringPool;

/**
 * Class: String
 *
 * Summary:
 * Represents a sequence of characters in memory, which need not be null
 * terminated in memory. It is immutable & modifications are done on copies of
 * strings.
 *
 * Strings can share character data (like substrings), to save memory and keep
 * consistency b/w them as they are immutable (unless you hack C++ objects and
 * get their kmalloc-memory location).
 *
 * Functions:
 * compareTo - Compares itself with another String object lexicographically
 * contains - Checks whether a substring exists in this
 * endsWith - Test if the String ends with the suffix given
 * getChars - Copies character from this string to a buffer given
 * hashCode - Used to get hash codes of null-terminated character-sequences & of this
 * startsWith - Tests if the String begins with the prefix given, at a offset
 * indexOf - Finds the first occurence of a character/substring in this/given sequence
 * isEmpty - Checks whether this is a empty-string (length == 0)
 * substring - Returns a substring bounded by indexes given
 * toUpperCase - Returns a string with all upper-case letters
 * toLowerCase - Returns a string with all lower-case letters
 *
 * Author: Shukant Pal
 */
class String final : public Object
{
public:
	String();
	String(const char *value);
	String(const char *value, unsigned int length);
	String(String& anotherString);
	virtual ~String();

	inline ULONG length(){ return (count); }
	static ULONG lengthOf(char *chSeq);

	inline char charAt(unsigned long index)
	{
		if(index < count)
			return (value[index]);
		else
			return (0);
	}

	int compareTo(String& anotherString);
//	String concat(String *subString);
	bool contains(String *subString);
	static int compare(String& firstString, String& secondString);
//	static String copyValueOf(CHAR dataBuf[], ULONG bufSize, ULONG copyStart);
	bool endsWith(String& suffix);
	bool getChars(unsigned int srcBegin, unsigned int srcEnd, char *dstBuffer);
	static unsigned int hashCode(const char *value);
	unsigned int hashCode();
	unsigned int indexOf(char chReq, unsigned int fromIndex = 0);
	unsigned int indexOf(String& subString, unsigned int fromIndex = 0);
	static unsigned int indexOf(const char *source, unsigned int sourceCount, const char *target, unsigned int targetCount);
	inline bool isEmpty(){ return (count == 0); }
	unsigned int lastIndexOf(char ch, unsigned int fromIndex = 0);
//	bool replace(char orgChar, char newChar);
	bool regionMatches(unsigned int thisOffset, String& otherString, unsigned int otherOffset, unsigned int length);
	bool startsWith(String& prefix, unsigned int offset=0);
	String *substring(unsigned int beginIndex, unsigned int endIndex);

//	String& toUpperCase();
//	String& toLowerCase();
	virtual String& toString();

	static String *get(char *ch_val);

	static void referTo(String& str);
	static void dispose(String& str);

private:
	unsigned int  count;// Number of characters in this string
	unsigned int offset;// Offset from kmalloc-source to this value
	StringPool *ownerPool;// Pool of strings in which this is registered, if any
	unsigned int hash;// Cache of hash, nice rhyme
	unsigned int referCount;// No. of references to this string
	const char *source;// kmalloc-Source of memory for this value
	const char *value;// Sequence of characters of this string

	String(const char *value, unsigned int offset, unsigned int count);// essentially shares the memory buffer

	friend class StringPool;
};

#endif /* MODULES_MODULEFRAMEWORK_INTERFACE_STRING_HXX_ */
