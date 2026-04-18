#include "GameTypes.h"
#include "GameAPI.h"

#include <string>
#include <algorithm>

/*** String ***/

// don't yet have implementation of the form alloc and free

String::String()
{
	//
}

String::~String()
{
	//
}

bool String::Set(const char * src)
{
	if (!src) {
		FormHeap_Free(m_data);
		m_data = 0;
		m_bufLen = 0;
		m_dataLen = 0;
		return true;
	}
	
	UInt32	srcLen = strlen(src);

	// realloc if needed
	if(srcLen > m_bufLen)
	{
		FormHeap_Free(m_data);
		m_data = (char *)FormHeap_Allocate(srcLen + 1);
		m_bufLen = m_data ? srcLen : 0;
	}

	if(m_data)
	{
		strcpy_s(m_data, m_bufLen + 1, src);
		m_dataLen = srcLen;
	}
	else
	{
		m_dataLen = 0;
	}

	return m_data != NULL;
}

bool ci_equal(char ch1, char ch2)
{
	return tolower((unsigned char)ch1) == tolower((unsigned char)ch2);
}

bool String::Includes(const char *toFind) const
{
	if (!m_data || !toFind)		//passing null ptr to std::string c'tor = CRASH
		return false;
	std::string curr(m_data, m_dataLen);
	std::string str2(toFind);
	std::string::iterator currEnd = curr.end();
	return (std::search(curr.begin(), currEnd, str2.begin(), str2.end(), ci_equal) != currEnd);
}

bool String::Replace(const char *_toReplace, const char *_replaceWith)
{
	if (!m_data || !_toReplace)
		return false;

	std::string curr(m_data, m_dataLen);
	std::string toReplace(_toReplace);

	std::string::iterator currBegin = curr.begin();
	std::string::iterator currEnd = curr.end();
	std::string::iterator replaceIt = std::search(currBegin, currEnd, toReplace.begin(), toReplace.end(), ci_equal);
	if (replaceIt != currEnd) {
		std::string replaceWith(_replaceWith);
		// we found the substring, now we need to do the modification
		std::string::size_type replacePos = distance(currBegin, replaceIt);
		curr.replace(replacePos, toReplace.size(), replaceWith);
		Set(curr.c_str());
		return true;
	}
	return false;
}

bool String::Append(const char* toAppend)
{
	std::string curr("");
	if (m_data)
		curr = std::string(m_data, m_dataLen);

	curr += toAppend;
	Set(curr.c_str());
	return true;
}

double String::Compare(const String& compareTo, bool caseSensitive)
{
	if (!m_data)
		return -2;		//signal value if comparison could not be made

	std::string first(m_data, m_dataLen);
	std::string second(compareTo.m_data, compareTo.m_dataLen);

	if (!caseSensitive)
	{
		std::transform(first.begin(), first.end(), first.begin(), tolower);
		std::transform(second.begin(), second.end(), second.begin(), tolower);
	}

	double comp = 0;
	if (first < second)
		comp = -1;
	else if (first > second)
		comp = 1;
	
	return comp;
}

// Modernization: std::string conversion support
String::operator std::string() const
{
	if (m_data)
		return std::string(m_data, m_dataLen);
	return std::string();
}

String::String(const std::string& str)
{
	m_data = NULL;
	m_dataLen = 0;
	m_bufLen = 0;
	Set(str.c_str());
}

// Modernization: Copy constructor
String::String(const String& other)
{
	m_data = NULL;
	m_dataLen = 0;
	m_bufLen = 0;
	if (other.m_data)
		Set(other.m_data);
}

// Modernization: Assignment operators
String& String::operator=(const String& other)
{
	if (this != &other)
		Set(other.m_data);
	return *this;
}

String& String::operator=(const char* src)
{
	Set(src);
	return *this;
}

String& String::operator=(const std::string& str)
{
	Set(str.c_str());
	return *this;
}

// Modernization: Comparison operators
bool String::operator==(const String& other) const
{
	if (m_dataLen != other.m_dataLen)
		return false;
	if (!m_data && !other.m_data)
		return true;
	if (!m_data || !other.m_data)
		return false;
	return strcmp(m_data, other.m_data) == 0;
}

bool String::operator==(const char* other) const
{
	if (!m_data && !other)
		return true;
	if (!m_data || !other)
		return false;
	return strcmp(m_data, other) == 0;
}

bool String::operator!=(const String& other) const
{
	return !(*this == other);
}

bool String::operator!=(const char* other) const
{
	return !(*this == other);
}

bool String::operator<(const String& other) const
{
	if (!m_data)
		return other.m_data != NULL;
	if (!other.m_data)
		return false;
	return strcmp(m_data, other.m_data) < 0;
}

// Modernization: Append operators
String& String::operator+=(const char* toAppend)
{
	Append(toAppend);
	return *this;
}

String& String::operator+=(const String& other)
{
	if (other.m_data)
		Append(other.m_data);
	return *this;
}
