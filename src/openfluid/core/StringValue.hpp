/*

  This file is part of OpenFLUID software
  Copyright(c) 2007, INRA - Montpellier SupAgro


 == GNU General Public License Usage ==

  OpenFLUID is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OpenFLUID is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OpenFLUID. If not, see <http://www.gnu.org/licenses/>.


 == Other Usage ==

  Other Usage means a use of OpenFLUID that is inconsistent with the GPL
  license, and requires a written agreement between You and INRA.
  Licensees for Other Usage of OpenFLUID may use this file in accordance
  with the terms contained in the written agreement between You and INRA.
  
*/



/**
  @file StringValue.hpp

  @author Jean-Christophe FABRE <jean-christophe.fabre@supagro.inra.fr>
 */


#ifndef __OPENFLUID_CORE_STRINGVALUE_HPP__
#define __OPENFLUID_CORE_STRINGVALUE_HPP__


#include <string>
#include <vector>

#include <openfluid/core/SimpleValue.hpp>

#include <openfluid/dllexport.hpp>



namespace openfluid { namespace core {
/**
StringValue is a container for a std::string value, with methods for conversion
to other containers derived from Value.\n

\see Value

\n

<I>Example : declaration</I>
@code
  // declaration of a StringValue, initialized to an empty string by default
  openfluid::core::StringValue Val1;

  // declaration of a StringValue, initialized to "hello world"
  openfluid::core::StringValue Val2("hello world");
@endcode


<I>Example : getting the contained value</I>
@code
  std::string Tmp1;

  // using the get method
  Tmp1 = Val1.get();

  // or using the cast operator
  Tmp1 = Val1;
@endcode


<I>Example : setting the contained value</I>
@code
  // using the set method
  Val1.set("Have a nice day");
@endcode
*/
class OPENFLUID_API StringValue : public SimpleValue
{
  private:

    std::string m_Value;

    static bool convertStringToDouble(const std::string& Str, double& Dbl);

    static std::vector<std::string> splitString(const std::string& StrToSplit,
                                                const std::string& Separators,
                                                bool ReturnsEmpty = false);

    std::vector<std::string> split(const std::string& Separators,
                                   bool ReturnsEmpty = false) const;


  public:

    /**
      Default constructor
    */
    StringValue() : m_Value("")
    { };

    /**
      Copy constructor
    */
    StringValue(const StringValue& Val) : SimpleValue(Val),
      m_Value(Val.m_Value)
    { };

    /**
      Constructor from char*
    */
    StringValue(const char* Val) : SimpleValue(), m_Value(std::string(Val))
    { };

    /**
      Constructor from std::string
    */
    StringValue(const std::string& Val) : SimpleValue(), m_Value(Val)
    { };

    /**
      Constructor from bool
    */
    StringValue(bool Val);

    /**
      Constructor from int
    */
    StringValue(int Val);

    /**
      Constructor from double
    */
    StringValue(double Val);

    /**
      Assignment operator
    */
    Value& operator=(const Value& Other);

    /**
      Cast operator
    */
    operator std::string() const
    { return m_Value; };

    virtual ~StringValue()
    { };

    inline Type getType() const
    { return Value::STRING; };

    Value* clone() const
    { return new StringValue(*this); };

    bool convert(Value& Val) const;

    /**
      Returns the string value as std::string type
      @return the string value
    */
    inline std::string get() const
    { return m_Value; };

    /**
      Returns a reference to the string value as std::string type
      @return the string value
    */
    inline std::string& data()
    { return m_Value; };

    /**
      Returns a reference to the string value as std::string type
      @return the string value
    */
    inline const std::string& data() const
    { return m_Value; };

    /**
      Sets the string value
      @param[in] Val the string value
    */
    inline void set(const std::string& Val)
    { m_Value = Val; };


    inline void clear()
    { m_Value.clear(); }


    void writeToStream(std::ostream& OutStm) const;

    void writeQuotedToStream(std::ostream& OutStm) const
    { OutStm << "\"" ; writeToStream(OutStm); OutStm << "\"" ; }

    /**
      Returns the size of the string
      @return size of the string
    */
    inline unsigned long getSize() const
    { return m_Value.size(); };

    /**
      Returns the size of the string
      @return size of the string
    */
    unsigned long size() const
    { return m_Value.size(); };


    /**
      Replaces all occurences of FindStr by ReplaceStr
      @param[in] FindStr the substring to find
      @param[in] ReplaceStr the substring to replace the found substrings
      @return the number of occurences
    */
    unsigned int replaceAll(const std::string& FindStr,const std::string& ReplaceStr);


    /**
      Try to find the the most adapted type for conversion
      @return the most adapted type for conversion (if the value is empty)
    */
    Value::Type guessTypeConversion() const;


    /**
      Converts the contained string to a double value (if possible)
      @return bool true if the conversion is correct, false otherwise
    */
    bool toDouble(double& Val) const;

    /**
      Converts the contained string to a DoubleValue (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toDoubleValue(DoubleValue& Val) const;

    /**
      Converts the contained string to a boolean value (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toBoolean(bool& Val) const;

    /**
      Converts the contained string to a BooleanValue (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toBooleanValue(BooleanValue& Val) const;

    /**
      Converts the contained string to an int value (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toInteger(int& Val) const;

    /**
      Converts the contained string to a long value (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toInteger(long& Val) const;

    /**
      Converts the contained string to an IntegerValue (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toIntegerValue(IntegerValue& Val) const;

    /**
      Converts the contained string to a NullValue (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toNullValue(NullValue& Val) const;

    /**
      Converts the contained string to a VectorValue value (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toVectorValue(VectorValue& Val) const;

    /**
      Converts the contained string to a MatrixValue value (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toMatrixValue(MatrixValue& Val) const;

    /**
      Converts the contained string to a MatrixValue value (if possible)
      @param[in] RowLength the size of a row
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toMatrixValue(const unsigned int& RowLength, MatrixValue& Val) const;

    /**
      Converts the contained string to a MapValue value (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
    */
    bool toMapValue(MapValue& Val) const;

    /**
      Converts the contained string to a TreeValue value (if possible)
      @param[out] Val the converted value
      @return bool true if the conversion is correct, false otherwise
      @throw FrameworkException when called
      @warning This method is currently not implement and always throws a FrameworkException
    */
    bool toTreeValue(TreeValue& Val) const;

};


} }  // namespaces



#endif /* __OPENFLUID_CORE_STRINGVALUE_HPP__ */
