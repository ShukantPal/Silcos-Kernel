/**
 * File: String.cxx
 *
 * Summary:
 * Implements String class and provides its allocation-information for using kernel
 * kobj-new operator.
 * ___________________________________________________________________
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
#include <Arrays.hxx>
#include <Math.hxx>
#include <Heap.hxx>
#include <String.hxx>
#include <KERNEL.h>

#define DEFAULT_HASH 0xFFFFFFFF

ObjectInfo* tString = 0;
char nmString[] = "com.silcos.mdfrwk.String";

const char nullValue[] = "String@null";

String *defaultString;// Used for object::toString

Object::~Object()
{
	// nothing done!!!
}

String& Object::toString()
{
	return (*defaultString);
}

String::String()
{
	this->count = 0;
	this->hash = DEFAULT_HASH;
	this->offset = 0;
	this->ownerPool = NULL;
	this->referCount = 1;
	this->value = nullValue;
	this->source = NULL;
}

/**
 * Function: String::String
 *
 * Summary:
 * Initializes a string from a null-terminated character sequence, that at longest
 * must be of 480 length. If greater, the string will have a length of 480.
 *
 * Args:
 * const char *value - null-terminated character sequence
 *
 * Author: Shukant Pal
 */
String::String(const char *value)
{
	this->count = String::lengthOf((char*) value);
	this->hash = DEFAULT_HASH;
	this->offset = 0;
	this->ownerPool = NULL;
	this->referCount = 1;
	this->value = (const char*) Arrays::copyOf((const void*) value, count);

	this->source = this->value;
}

/**
 * Function: String::String
 *
 * Summary:
 * Initializes a string from character array of a given length by copying its
 * contents into private, kmalloc-memory.
 *
 * Args:
 * const char *value_ - pointer to character array
 * unsigned int length - length of this string
 *
 * Author: Shukant Pal
 */
String::String(const char *value_, unsigned int length)
{
	this->count = length;
	this->hash = DEFAULT_HASH;
	this->offset = 0;
	this->ownerPool = NULL;
	this->referCount = 1;
	this->value = (const char*) Arrays::copyOf((const void *) value_, length);

	this->source = value;
}

/**
 * Function: String::String
 *
 * Summary:
 * Initializes a string from a already-string memory buffer at the offset given. It
 * is private to prevent users from initializing a String with mutable memory.
 *
 * Args:
 * const char *value - source for the kmalloc-memory (not value for this string)
 * unsigned int offset - offset of this string from source given
 * unsigned int length - length of this string in characters
 *
 * Author: Shukant Pal
 */
String::String(const char *value, unsigned int offset, unsigned int length)
{
	this->count = length;
	this->offset = offset;
	this->ownerPool = NULL;
	this->hash = DEFAULT_HASH;
	this->referCount = 1;
	this->source = value;
	this->value = value + offset;

	kuse((const void*) value);// Reinforce a new-user to kmalloc-memory
}

/**
 * Function: String::String
 *
 * Summary:
 * Copies the string data to this & shares the character contents.
 */
String::String(String& anotherString)
{
	this->count = anotherString.count;
	this->offset = anotherString.offset;
	this->ownerPool = anotherString.ownerPool;
	this->hash = DEFAULT_HASH;
	this->referCount = 1;
	this->source = anotherString.source;
	this->value = anotherString.value;

	kuse((const void *) anotherString.source);
}

String::~String()
{
	// Some strings' value are stored in non-kmalloc regions thus, not all
	// can be freed.
	if(source != NULL)
		kfree((void*) source);
}

bool String::endsWith(String& suffix)
{
	return startsWith(suffix, count - suffix.count);
}

/**
 * Function: String::lengthOf
 *
 * Summary;
 * Returns the number of characters in the sequence, coming before a null-terminator
 * and doesn't stop unless the index goes above 480.
 *
 * Args:
 * char *chSeq - Pointer to the character buffer
 *
 * Author: Shukant Pal
 */
unsigned long String::lengthOf(char *chSeq)
{
	for(long index=0; index<480; index++)
	{
		if(*chSeq == '\0')
			return (index);

		++(chSeq);
	}

	return (0);
}

/**
 * Function: String::compare
 *
 * Summary:
 * Compares to string lexicographically, and is identical to the following -
 * 					firstString.compareTo(secondString)
 *
 * Args:
 * String& firstString, secondString - Strings to compare
 *
 * See: String::compareTo
 *
 * Author: Shukant Pal
 */
int String::compare(String& firstString, String& secondString)
{
	return firstString.compareTo(secondString);
}

/**
 * Function: String::compareTo
 *
 * Summary:
 * Compares this string with the given string lexicographically.
 *
 * Args:
 * String& anotherString - String to compare this with
 *
 * Returns:
 * positive integer if this string comes after the other lexicographically; a
 * negative integer if it comes before the other string; 0 if they are identical.
 *
 * Author: Shukant Pal
 */
int String::compareTo(String& anotherString)
{
	int len1 = count;
	int len2 = anotherString.count;
	int srhCount = Math::min(len1, len2);

	const char *val1 = value;
	const char *val2 = anotherString.value;

	for(; srhCount > 0; srhCount--)
	{
		if(*val1 != *val2){
			return (*val1 - *val2);
		}
		++(val1);
		++(val2);
	}

	return (len1 - len2);
}

/**
 * Function: String::getChars
 *
 * Summary:
 * Copies the contents of this string from index srcBegin to index srcEnd into
 * the destination buffer (provided that it is a valid location). The size of the copy
 * is -
 * 						srcEnd - srcBegin + 1
 *
 * Args:
 * unsigned int srcBegin - index of first character to copy
 * unsigned int srcEnd - index of last character to copy
 * char *dst - Buffer into which contents are to be copied
 *
 * Author: Shukant Pal
 */
bool String::getChars(unsigned int srcBegin, unsigned int srcEnd, char *dst)
{
	if(srcEnd >= count || srcEnd <= srcBegin)
		return (false);

	Arrays::copy((const void*) value + srcBegin, dst, srcEnd - srcBegin + 1);

	return (true);
}

/**
 * Function: String::startsWith
 *
 * Summary:
 * Tests whether this string starts with the substring 'prefix' given at a
 * given offset.
 *
 * Args:
 * String& prefix - Substring, whose existence to be tested
 * unsigned int offset - Offset in this string, to start the test
 *
 * Returns:
 * true, if the substring at offset starts with prefix; false, if they don't
 * match, or if offset is greater than count.
 *
 * Author: Shukant Pal
 */
bool String::startsWith(String& prefix, unsigned int offset)
{
	if(offset > count - prefix.count)
		return (false);

	const char *tChar = value + offset;
	const char *pChar = prefix.value;

	unsigned int counter = prefix.count;
	while(--counter >= 0)
	{
		if(*tChar != *pChar)
			return (false);

		++(tChar);
		++(pChar);
	}

	return (true);
}

/**
 * Function: String::subString
 *
 * Summary:
 * Returns the substring of this string which starts from the character at
 * beginIndex and ends at the character at endIndex. The substring uses the same
 * character data as this string.
 *
 * The resulting size of the substring would be - (endIndex - beginIndex + 1)
 *
 * Args:
 * unsigned int beginIndex - Index of the first character in this string, from which
 * the substring should start.
 *
 * unsigned int endIndex - Index of the last character in this string, from which the
 * substring should end.
 *
 * Returns:
 * A substring which start with the character at beginIndex & ends with the
 * character at endIndex, if beginIndex & endIndex are less than this strings count
 * and endIndex is greater than beginIndex; otherwise, NULL.
 *
 * Author: Shukant Pal
 */
String *String::substring(unsigned int beginIndex, unsigned int endIndex)
{
	if(beginIndex >= count || endIndex >= count || endIndex <= beginIndex)
	{
		return (NULL);
	}
	else
	{
		return new(tString) String(value, beginIndex, endIndex - beginIndex + 1);
	}
}

#define FNV_PRIME 0x811C9DC5

unsigned int String::hashCode(const char *value)
{
	unsigned int hashValue = 0;

	while(*value)
	{
		hashValue *= FNV_PRIME;
		hashValue ^= *value;

		++(value);
	}

	return (hashValue);
}

unsigned int String::hashCode()
{
	if(hash != DEFAULT_HASH)
		return (hash);

	unsigned int hashValue = 0;
	const char *indexedChar = value;

	for(unsigned int i = 0; i < count; i++)
	{
		hashValue *= FNV_PRIME;
		hashValue ^= *indexedChar;

		++(indexedChar);
	}

	this->hash = hashValue;

	return (hashValue);
}

/**
 * Function: String::indexOf
 *
 * Summary;
 * Returns the index of the first occurence of ch in this string,
 * starting the search at fromIndex (optional); if the character is not  found,
 * then -1 is returned.
 *
 * Args:
 * char ch -  Character to find
 * unsigned int fromIndex - Index to start the search from
 *
 * Returns:
 * Index of the first occurence of ch in this string, after fromIndex; if not
 * found, (unsigned int) -1;
 *
 * Author: Shukant Pal
 */
unsigned int String::indexOf(char ch, unsigned int fromIndex)
{
	if(fromIndex >= count)
		return (-1);

	unsigned int schCounter = count - fromIndex + 1;
	const char *tCharacter = value + fromIndex;
	while(schCounter-- > 0)
	{
		if((char) *tCharacter == ch)
			return (fromIndex + count - schCounter - 1);

		++(tCharacter);
	}

	return (-1);
}

unsigned int String::indexOf(String& searchPhrase, unsigned int fromIndex)
{
	return indexOf(value + fromIndex, count, searchPhrase.value, searchPhrase.count);
}

/**
 * Function: String::indexOf
 *
 * Summary:
 * Returns the index of the first occurence of target in the source. It works
 * by  searching for the first character of target in source & on each occurence,
 * testing whether the substring is located there. If it is not found, (unsigned
 * int) -1 is returned.
 *
 * Args:
 * const char *source - Sequence of characters to search the substring in
 * unsigned int sourceCount - Count of characters in source
 * const char *target - Sequence of characters representing the substring
 * unsigned int targetCount - Count of characters in target
 *
 * Returns:
 * the index of the first occurence of target in source, if it exists; otherwise,
 * (unsigned int) - 1;
 *
 * Author: Shukant Pal
 */
unsigned int String::indexOf(const char *source, unsigned int sourceCount, const char *target,
				unsigned int targetCount)
{
	char first = *target;
	unsigned int max = sourceCount - targetCount;

	for(unsigned int i = 0; i < max; i++)
	{
		if(source[i] != first)
		{
			while(++i <= max && source[i] != first){}
		}

		if(i <= max)
		{
			int sidx = i + 1;
			int endx = sidx + targetCount - 1;
			for(unsigned int tidx = 1;
					sidx < endx && source[sidx] == target[tidx];
					sidx++, tidx++)
			{}

			if(sidx == endx)
				return (i);
		}
	}

	return (-1);
}

/**
 * Function: String::lastIndexOf
 *
 * Summary:
 * Returns the last index of the ch before fromIndex (optional) in this string,
 * and if the character is not found, -1 is returned.
 *
 * Args:
 * char ch - Character to search in this string
 * unsigned int fromIndex - Index from which search is to start, backward
 *
 * Returns:
 * last index of ch in this string before fromIndex, if it occurs; -1, if it doesn't
 * or fromIndex is out of bounds.
 *
 * Author: Shukant Pal
 */
unsigned int String::lastIndexOf(char ch, unsigned int fromIndex)
{
	if(fromIndex >= count)
		return (-1);

	unsigned int schCounter = fromIndex + 1;
	const char *tchar = value +  fromIndex;
	while(schCounter--  > 0)
	{
		if(*tchar == ch)
		{
			return (schCounter - 1);
		}
		--(tchar);
	}

	return (-1);
}

/**
 * Function: String::regionMatches
 *
 * Summary:
 * Tests whether the substring at thisOffset of length given matches the substring
 * of the other string argument given at otherOffset.
 *
 * Args:
 * unsigned int thisOffset - Offset of the substring in this string
 * String& otherString - Other string, whose substring is to be matched with
 * unsigned int otherOffset - Offset of substring in other string
 * unsigned int length - Length of the substrings
 *
 * Returns:
 * true, if both substring match; false, if they don't match or if atleast one substring
 * given doesn't exist in the bounds given (index & length).
 *
 * Author: Shukant Pal
 */
bool String::regionMatches(unsigned int thisOffset, String& otherString, unsigned int otherOffset,
				unsigned int length)
{
	if(thisOffset + length > count || otherOffset + length > count ||
							count > 0)
		return (false);

	while(length-- > 0)
	{
		if(value[thisOffset++] != otherString.value[otherOffset++])
			return (false);
	}

	return (true);
}

/**
 * Function: String::toString
 *
 * Summary:
 * Returns this string, because it represents itself. Don't rely on toString's
 * return value as it may be changed in future API versions, and may contain
 * a @mdfrwk.String$ prefix.
 *
 * Author: Shukant Pal
 */
String& String::toString()
{
	return (*this);
}
