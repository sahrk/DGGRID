/*******************************************************************************
    Copyright (C) 2023 Kevin Sahr

    This file is part of DGGRID.

    DGGRID is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DGGRID is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
//
// DgApParamList.h: DgApParamList class definitions
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DGAPPARAMLIST_H
#define DGAPPARAMLIST_H

#include <dglib/DgBase.h>
#include <dglib/DgString.h>
#include <dglib/DgUtil.h>

#include <cfloat>
#include <climits>
#include <string>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
class DgApAssoc {

   public:

      DgApAssoc (conststd::string& nameIn)
         : isApplicable_(false), isValid_(false), isDefault_(true),
           isUserSet_(false), isUsed_(false)
         { name_ = toLower(nameIn); }

      virtual ~DgApAssoc (void);

      virtualstd::string valToStr (void) const = 0;

      conststd::string& name (void) const { return name_; }

     std::string asString (void) const
           { return name() + " " + (isValid() ? valToStr() :std::string("INVALID"))
                    + " (" +
                       (!isApplicable() ?std::string("N/A") :
                         (isDefault() ?std::string("default") :std::string("user set")))
                    + ")"; }

     std::string validationErrMsg (void) const { return validationErrMsg_; }

      void setValidationErrMsg (conststd::string& valErrMsgIn)
               { validationErrMsg_ = valErrMsgIn; }

      bool isApplicable (void) const { return isApplicable_; }

      bool setIsApplicable (bool isApplicableIn) { return (isApplicable_ = isApplicableIn); }

      bool isValid (void) const { return isValid_; }

      bool setIsValid (bool isValidIn) { return (isValid_ = isValidIn); }

      bool isDefault (void) const { return isDefault_; }

      bool setIsDefault (bool isDefaultIn) { return (isDefault_ = isDefaultIn); }

      bool isUserSet (void) const { return isUserSet_; }

      bool setIsUserSet (bool isUserSetIn) { return (isUserSet_ = isUserSetIn); }

      bool isUsed (void) const { return isUsed_; }

      bool setIsUsed (bool isUsedIn) { return (isUsed_ = isUsedIn); }

      virtual void setValStr (conststd::string& valStr) = 0;

      DgApAssoc& operator= (const DgApAssoc& obj)
        {
           if (&obj != this) {
              name_ = obj.name();
              validationErrMsg_ = obj.validationErrMsg();
              isValid_ = obj.isValid();
           }

           return *this;
        }

      virtual bool validate (void) { return (isValid_ = true); }

   protected:

     std::string name_;
     std::string validationErrMsg_;

      bool isApplicable_;
      bool isValid_;
      bool isDefault_;
      bool isUserSet_;
      bool isUsed_;
};

////////////////////////////////////////////////////////////////////////////////
inline ostream& operator<< (ostream& stream, const DgApAssoc& assoc)
{
   stream << assoc.asString() << endl;

   return stream;

} // inline ostream& operator<<

////////////////////////////////////////////////////////////////////////////////
class DgApParamList {

   public:

      DgApParamList (void) { }

     ~DgApParamList (void);

      vector<DgApAssoc*> parameters;

      void clearList (void);

      void loadParams (conststd::string& fileName, bool fail = true);

      void setParam (conststd::string& nameIn, conststd::string& strValIn, bool fail = true);

      void setPresetParam (conststd::string& nameIn, conststd::string& strValIn);

      void insertParam (DgApAssoc* param); // does not make a copy

      DgApAssoc* getParam (conststd::string& nameIn,
                         bool setToIsApplicable = true) const;

};

////////////////////////////////////////////////////////////////////////////////
inline ostream& operator<< (ostream& stream, const DgApParamList& plist)
{
   for (unsigned int i = 0; i < plist.parameters.size(); i++) {
      if (plist.parameters[i]->isUsed())
         stream << *(plist.parameters[i]);
   }

   return stream;

} // inline ostream& operator<<

////////////////////////////////////////////////////////////////////////////////
template<class T> class DgParameter : public DgApAssoc {

   public:

      DgParameter<T> (conststd::string& nameIn) : DgApAssoc (nameIn) { }

      DgParameter<T> (conststd::string& nameIn, const T& valIn, bool validIn = true)
        : DgApAssoc (nameIn), value_ (valIn) { DgApAssoc::setIsValid(validIn); }

      const T& value (void) const { return value_; }

      virtual void setValue (const T& value) { value_ = value; validate(); }

      virtual void setValStr (conststd::string& valStr)
        {
           setValue(strToVal(valStr));
        }

      virtualstd::string valToStr (void) const = 0;
      virtual T strToVal (conststd::string& strVal) const = 0;

      DgParameter<T>& operator= (const DgParameter<T>& obj)
        {
           if (&obj != this) {
              DgApAssoc::operator=(obj);
              value_ = obj.value();
           }

           return *this;
        }

      DgParameter<T>& operator= (const DgApAssoc& obj)
      {
         const DgParameter<T>* objT = dynamic_cast<const DgParameter<T>*>(&obj);
         if (!objT) {
            report(
           std::string("DgParameter::operator=() conflicting types for parameter "
                     + name(), DgBase::Fatal));
         }

         if (objT != this) {
            DgApAssoc::operator=(*objT);
            value_ = objT->value();
         }

         return *this;
      }

   protected:

      T value_;
};

////////////////////////////////////////////////////////////////////////////////
template<class T> bool getParamValue (const DgApParamList& plist,
                                      conststd::string& name, T& var,
                                      bool dieOnFail = true)
{
   DgApAssoc* assoc = plist.getParam(name);
   if (!assoc) {
      if (dieOnFail) {
         report(std::string("getParamValue() missing required parameter ") + name,
                DgBase::Fatal);
      } else {
         return false;
      }
   }

   const DgParameter<T>* assocT = dynamic_cast<const DgParameter<T>*>(assoc);
   if (!assocT) {
      if (dieOnFail) {
         report(std::string("getParamValue() type mismatch on parameter ") + name,
                DgBase::Fatal);
      } else {
         return false;
      }
   }

   // if we're here everything is peachy

   assoc->setIsUsed(true);
   var = assocT->value();

   return true;

} // template<class T> bool getParamValue

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class DgStringParam : public DgParameter<string> {

   public:

      DgStringParam (conststd::string& nameIn)
         : DgParameter<string> (nameIn), strip_ (false) { }

      DgStringParam (conststd::string& nameIn, conststd::string& valIn,
                     bool validIn = true, bool stripIn = true)
        : DgParameter<string> (nameIn, valIn, validIn), strip_ (stripIn) { }

      virtual void setValue (conststd::string& val)
       { DgParameter<string>::setValue(
              ((strip_) ? dgg::util::stripQuotes(val) : val)); }

      virtualstd::string valToStr (void) const { return value_; }
      virtualstd::string strToVal (conststd::string& strVal) const { return strVal; }

      private:
         bool strip_;
};

////////////////////////////////////////////////////////////////////////////////
class DgBoolParam : public DgParameter<bool> {

   public:

      DgBoolParam (conststd::string& nameIn) : DgParameter<bool> (nameIn) { }

      DgBoolParam (conststd::string& nameIn, bool valIn, bool validIn = true)
        : DgParameter<bool> (nameIn, valIn, validIn) { }

      virtualstd::string valToStr (void) const
                        { returnstd::string(value() ? "true" : "false"); }

      virtual bool strToVal (conststd::string& strVal) const
          {
             DgBoolParam* me = const_cast<DgBoolParam*>(this);
            std::string lower = toLower(strVal);
             me->setIsValid(true);

             if (lower ==std::string("true")) return true;
             else if (lower ==std::string("false")) return false;

             // if we're here we have a bad value

             me->setIsValid(false);

             me->setValidationErrMsg(std::string("Value '") + strVal +
              std::string("' is not one of the allowed values 'true' or 'false'"));

             return false;
          }

      virtual bool validate (void) { return isValid(); }
};

////////////////////////////////////////////////////////////////////////////////
template<class T>
class DgBoundedParam : public DgParameter<T> {

   public:

      DgBoundedParam (conststd::string& nameIn) : DgParameter<T> (nameIn) { }

      DgBoundedParam (conststd::string& nameIn, const T& valIn, const T& minIn,
                      const T& maxIn, bool validIn = true)
       : DgParameter<T> (nameIn, valIn, validIn), min_ (minIn), max_ (maxIn) { }

      DgBoundedParam (conststd::string& nameIn, const T& minIn, const T& maxIn)
          : DgParameter<T> (nameIn), min_ (minIn), max_ (maxIn) { }

      virtual bool validate (void)
                {
                   DgApAssoc::setIsValid((this->value() >= min()) && (this->value() <= max()));
                   return this->isValid();
                }

      DgBoundedParam& operator= (const DgBoundedParam& obj)
               {
                  if (&obj != this) {
                     DgParameter<T>::operator=(obj);
                     min_ = obj.min();
                     max_ = obj.max();
                  }

                  return *this;

               } // DgBoundedParam::operator=

      // get methods

      const T& min (void) const { return min_; }
      const T& max (void) const { return max_; }

      // set methods

      void setMin (const T& minIn) { min_ = minIn; }
      void setMax (const T& maxIn) { max_ = maxIn; }

   private:

      T min_;
      T max_;

}; // class DgBoundedParam

////////////////////////////////////////////////////////////////////////////////
class DgIntParam : public DgBoundedParam<int> {

   public:

     DgIntParam (conststd::string& nameIn, int minIn = INT_MIN,
                 int maxIn = INT_MAX)
          : DgBoundedParam<int> (nameIn, minIn, maxIn) { }

      DgIntParam (conststd::string& nameIn, const int& valIn,
                  const int& minIn = INT_MIN, const int& maxIn = INT_MAX,
                  bool validIn = true)
        : DgBoundedParam<int> (nameIn, valIn, minIn, maxIn, validIn)
                {
                  if (!validate()) {
                     report(
                       std::string("Invalid initialization data for parameter:\n")
                        + name() + " " + valToStr() +std::string("\n") +
                        validationErrMsg(), DgBase::Fatal);
                  }
                }

      virtualstd::string valToStr (void) const { return dgg::util::to_string(value_); }
      virtual int strToVal (conststd::string& strVal) const
                {
			return dgg::util::from_string<int>(strVal);
                }

      virtual bool validate (void)
                {
                   DgBoundedParam<int>::validate();
                   if (!isValid()) {
                      setValidationErrMsg(std::string("value out of range ") +
                                  dgg::util::to_string(min()) + " to " +
                                  dgg::util::to_string(max()));
                   }
                   return isValid();
                }
};

////////////////////////////////////////////////////////////////////////////////
class DgLIntParam : public DgBoundedParam<long long int> {

   public:

     DgLIntParam (conststd::string& nameIn,
                  long long int minIn = LLONG_MIN,
                  long long int maxIn = LLONG_MAX)
          : DgBoundedParam<long long int> (nameIn, minIn, maxIn) { }

      DgLIntParam (conststd::string& nameIn, const long long int& valIn,
                  const long long int& minIn = LLONG_MIN,
		  const long long int& maxIn = LLONG_MAX,
                  bool validIn = true)
        : DgBoundedParam<long long int> (nameIn, valIn, minIn, maxIn, validIn)
                {
                  if (!validate()) {
                     report(
                       std::string("Invalid initialization data for parameter:\n")
                        + name() + " " + valToStr() +std::string("\n") +
                        validationErrMsg(), DgBase::Fatal);
                  }
                }

      virtualstd::string valToStr (void) const { return dgg::util::to_string(value_); }
      virtual long long int strToVal (conststd::string& strVal) const
                {
			return dgg::util::from_string<long long int>(strVal);
                }

      virtual bool validate (void)
                {
                   DgBoundedParam<long long int>::validate();
                   if (!isValid()) {
                      setValidationErrMsg(std::string("value out of range ") +
                                  dgg::util::to_string(min()) + " to " + dgg::util::to_string(max()));
                   }
                   return isValid();
                }
};

////////////////////////////////////////////////////////////////////////////////
class DgULIntParam : public DgBoundedParam<unsigned long int> {

   public:

     DgULIntParam (conststd::string& nameIn, unsigned long int minIn = 0UL,
                 unsigned long int maxIn = ULONG_MAX)
          : DgBoundedParam<unsigned long int> (nameIn, minIn, maxIn) { }

      DgULIntParam (conststd::string& nameIn, const unsigned long int& valIn,
                  const unsigned long int& minIn = 0UL,
                  const unsigned long int& maxIn = ULONG_MAX,
                  bool validIn = true)
        : DgBoundedParam<unsigned long int>
                          (nameIn, valIn, minIn, maxIn, validIn)
                {
                  if (!validate()) {
                     report(
                       std::string("Invalid initialization data for parameter:\n")
                        + name() + " " + valToStr() +std::string("\n") +
                        validationErrMsg(), DgBase::Fatal);
                  }
                }

      virtualstd::string valToStr (void) const { return dgg::util::to_string(value_); }
      virtual unsigned long int strToVal (conststd::string& strVal) const
                      { return dgg::util::from_string<unsigned long int>(strVal); }

      virtual bool validate (void)
                {
                   DgBoundedParam<unsigned long int>::validate();
                   if (!isValid()) {
                      setValidationErrMsg(std::string("value out of range ") +
                                  dgg::util::to_string(min()) + " to " + dgg::util::to_string(max()));
                   }
                   return isValid();
                }
};

////////////////////////////////////////////////////////////////////////////////
class DgUint64Param : public DgBoundedParam<unsigned long long int> {

   public:

      DgUint64Param (conststd::string& nameIn, unsigned long long int minIn = 0ULL,
                   unsigned long long int maxIn = ULLONG_MAX)
          : DgBoundedParam<unsigned long long int> (nameIn, minIn, maxIn) { }

      DgUint64Param (conststd::string& nameIn, const unsigned long long int& valIn,
                    const unsigned long long int& minIn = 0ULL,
                    const unsigned long long int& maxIn = ULLONG_MAX,
                    bool validIn = true)
        : DgBoundedParam<unsigned long long int> (nameIn, valIn, minIn, maxIn, validIn)
                {
                  if (!validate()) {
                     report(
                       std::string("Invalid initialization data for parameter:\n")
                        + name() + " " + valToStr() +std::string("\n") +
                        validationErrMsg(), DgBase::Fatal);
                  }
                }

      virtualstd::string valToStr (void) const { return dgg::util::to_string(value_); }
      virtual unsigned long long int strToVal (conststd::string& strVal) const
                      { return dgg::util::from_string<unsigned long long int>(strVal); }

      virtual bool validate (void)
                {
                   DgBoundedParam<unsigned long long int>::validate();
                   if (!isValid()) {
                      setValidationErrMsg(std::string("value out of range ") +
                              dgg::util::to_string(min()) + " to " + dgg::util::to_string(max()));
                   }
                   return isValid();
                }
};

////////////////////////////////////////////////////////////////////////////////
class DgDoubleParam : public DgBoundedParam<long double> {

   public:

      DgDoubleParam (conststd::string& nameIn, long double minIn = LDBL_MIN,
                     long double maxIn = LDBL_MAX)
          : DgBoundedParam<long double> (nameIn, minIn, maxIn) { }

      DgDoubleParam (conststd::string& nameIn, const long double& valIn,
                     const long double& minIn = LDBL_MIN,
                     const long double& maxIn = LDBL_MAX, bool validIn = true)
          : DgBoundedParam<long double> (nameIn, valIn, minIn, maxIn, validIn)
                {
                  if (!validate()) {
                     report(
                       std::string("Invalid initialization data for parameter:\n")
                        + name() + " " + valToStr() +std::string("\n") +
                        validationErrMsg(), DgBase::Fatal);
                  }
                }

      virtualstd::string valToStr (void) const { return dgg::util::to_string(value_); }
      virtual long double strToVal (conststd::string& strVal) const
                      { return dgg::util::from_string<long double>(strVal); }

      virtual bool validate (void)
                {
                   DgBoundedParam<long double>::validate();
                   if (!isValid()) {
                      setValidationErrMsg(std::string("value out of range ") +
                                  dgg::util::to_string(min()) + " to " +
                                  dgg::util::to_string(max()));
                   }
                   return isValid();
                }
};

////////////////////////////////////////////////////////////////////////////////
template<class T>
class DgChoiceParam : public DgParameter<T> {

   public:

      DgChoiceParam (conststd::string& nameIn, const vector<T*>* choicesIn = 0)
          : DgParameter<T> (nameIn) { if (choicesIn) addChoices(*choicesIn); }

      DgChoiceParam (conststd::string& nameIn, const T& valIn,
                     const vector<T*>* choicesIn = 0, bool validIn = true)
        : DgParameter<T> (nameIn, valIn, validIn)
      {
          if (choicesIn)
           addChoices(*choicesIn);
      }

     ~DgChoiceParam (void)
           { clearChoices(); }

      const vector<T*>& choices (void) const { return choices_; }

      void addChoices (const vector<T*>& choicesIn) // makes copy
            {
               for (unsigned int i = 0; i < choicesIn.size(); i++) {
                  choices_.push_back(newstd::string(*choicesIn[i]));
               }
            }

      void clearChoices (void)
            {
		dgg::util::release(choices_);
            }

      virtualstd::string valToStr (void) const = 0;
      virtual T strToVal (conststd::string& strVal) const = 0;

      virtual bool validate (void)
                {
                   for (unsigned int i = 0; i < choices_.size(); i++) {
                      if (*choices_[i] == this->value())
		       return this->setIsValid(true);
                   }

                   this->setValidationErrMsg(std::string("value not allowed"));

                   return this->setIsValid(false);
                }

      DgChoiceParam& operator= (const DgChoiceParam& obj)
               {
                  if (&obj != this) {
                     DgParameter<T>::operator=(obj);

                     clearChoices();

                     for (unsigned int i = 0; i < obj.choices().size(); i++)
                        choices_.push_back(new T(*obj.choices()[i]));
                  }

                  return *this;

               } // DgChoiceParam::operator=

   protected:

      vector<T*> choices_;

}; // class DgChoiceParam

////////////////////////////////////////////////////////////////////////////////
class DgStringChoiceParam : public DgChoiceParam<string> {

   public:

      DgStringChoiceParam (conststd::string& nameIn,
                           const vector<string*>* choicesIn = 0)
          : DgChoiceParam<string> (nameIn, choicesIn)
                {
                   for (unsigned int i = 0; i < choices_.size(); i++) {
                      *choices_[i] = toLower(*choices_[i]);
                   }
                }

      DgStringChoiceParam (conststd::string& nameIn, conststd::string& valIn,
                     const vector<string*>* choicesIn = 0, bool validIn = true)
          : DgChoiceParam<string> (nameIn, valIn, choicesIn, validIn)
                {
                   for (unsigned int i = 0; i < choices_.size(); i++) {
                      *choices_[i] = toLower(*choices_[i]);
                   }

                   if (!validate()) {
                     report(
                       std::string("Invalid initialization data for parameter:\n")
                        + name() + " " + valToStr() +std::string("\n") +
                        validationErrMsg(), DgBase::Fatal);
                   }
                }

      virtualstd::string valToStr (void) const { return value_; }
      virtualstd::string strToVal (conststd::string& strVal) const { return strVal; }

      virtual bool validate (void)
                {
                  std::string lower = toLower(value());
                   for (unsigned int i = 0; i < choices_.size(); i++) {
                      if (*choices_[i] == lower)
		       return this->setIsValid(true);
                   }

                  std::string err(std::string("Value '") + value() +
                       std::string("' is not one of the allowed values:\n"));
                   for (unsigned int i = 0; i < choices_.size(); i++) {
                      err = err + *choices_[i] +std::string("\n");
                   }

                   setValidationErrMsg(err);

                   return this->setIsValid(false);
                }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif
