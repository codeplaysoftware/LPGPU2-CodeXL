// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGUtilVariantType interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_CCVG_UTIL_VARIANTTYPE_H_INCLUDE
#define LPGPU2_CCVG_UTIL_VARIANTTYPE_H_INCLUDE

/// @brief    CCVGUtilVariantType utility common class. The class impliements
///           the behaviour of a variant object which holds any data of type T.
///           A copy of the data object is made and stored in *this wrapper.
///           When *this object is destroyed the data object held within is
///           also destroyed calling the object's destructor should it have 
///           one.
/// @warning  None.
/// @see      None.
/// @date     17/08/2016.
/// @author   Illya Rudkin.
// clang-format off
class CCVGUtilVariantType
{
// Methods:
public:
  /* ctor */  CCVGUtilVariantType();
  /* ctor */  CCVGUtilVariantType(CCVGUtilVariantType &vrOther);
  /* ctor */  CCVGUtilVariantType(CCVGUtilVariantType &&vrOther);
  /* dtor */  ~CCVGUtilVariantType();
  //
  template<typename T> bool Set(const T &vrArg);
  template<typename T> T*   Get() const;
  //
  CCVGUtilVariantType& operator=(const CCVGUtilVariantType &vrOther);
  CCVGUtilVariantType& operator=(CCVGUtilVariantType &&vrOther);

// Classes:
private:
  /// @brief    Base class wrapper to the derived wrapper class holding the
  ///           the data object when assigned to it by the Set() function. Do
  ///           not use *this base class CDataObjBase directly to create 
  ///           objects. Use only the CDataObjBase derived classes. 
  /// @warning  None.
  /// @see      Class CDataObj.
  class CDataObjBase
  {
  // Methods:
  public:
    /* ctor */  CDataObjBase();
    /* ctor */  CDataObjBase(CDataObjBase &vrOther);
    /* ctor */  CDataObjBase(CDataObjBase &&vrOther);
    //
    CDataObjBase& operator=(const CDataObjBase &vrOther);
    CDataObjBase& operator=(CDataObjBase &&vrOther);

  // Overrideable:
  public:
    /* dtor */ virtual    ~CDataObjBase();
    virtual CDataObjBase* CreateCopyOfSelf();
    virtual bool          GetIsDerivedClass() const;

  // Overrideable:
  protected:
    virtual void Copy(const CDataObjBase &vrOther);
    virtual void Destroy();
  };

  /// @brief    Derived from CDataObjBase, *this class is the wrapper for the
  ///           object an aggregate of type T. The data object wrapped is a 
  ///           copy of the object assigned to the variant object. 
  /// @warning  None.
  /// @see      Class CDataObjBase.
  template<typename T>
  class CDataObj : public CDataObjBase
  {
  // Methods:
  public:
    /* ctor */  CDataObj();
    /* ctor */  CDataObj(const T &vrArg);
    /* ctor */  CDataObj(T &vrArg);
    /* ctor */  CDataObj(CDataObj &vrOther);
    /* ctor */  CDataObj(CDataObj &&vrOther);
    //
    CDataObj& operator=(const CDataObj &vrOther);
    CDataObj& operator=(CDataObj &&vrOther);
    //
    T& GetDataObject();

  // Overidden:
  public:
    // From CDataObjBase
    /* dtor */ virtual ~CDataObj() override;
    virtual CDataObjBase* CreateCopyOfSelf() override;
    virtual bool          GetIsDerivedClass() const override;

  // Overrideable:
  private:
    virtual void Duplicate(const CDataObj &vrOther);

  // Overridden:
  private:
    // From CDataObjBase
    virtual void Destroy() override;

  // Attributes:
  private:
    T m_dataObj;
  };

// Methods:
private:
  void Destroy();
  void Copy(const CCVGUtilVariantType &vrOther);

// Attributes:
private:
  CDataObjBase * m_pDataObjectWrapper;
};
// clang-format on

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @brief CDataObj constructor.
/// @param T The object's type.
template<typename T>
CCVGUtilVariantType::CDataObj<T>::CDataObj()
{
}

/// @brief     CDataObj constructor.
/// @param     T The object's type.
/// @param[in] vrArg The data object to be stored in the variant object.
template<typename T>
CCVGUtilVariantType::CDataObj<T>::CDataObj(const T &vrArg)
{
  m_dataObj = vrArg;
}

/// @brief     CDataObj constructor.
/// @param     T The object's type.
/// @param[in] vrArg The data object to be stored in the variant object.
template<typename T>
CCVGUtilVariantType::CDataObj<T>::CDataObj(T &vrArg)
{
  m_dataObj = vrArg;
}

/// @brief CDataObj destructor.
/// @param T The object's type.
template<typename T>
CCVGUtilVariantType::CDataObj<T>::~CDataObj()
{
  Destroy();
}

/// @brief  Retrieve the data object held by *this object wrapper.
/// @param  T  The object's type.
/// @return T& Reference to the data object
template<typename T>
T& CCVGUtilVariantType::CDataObj<T>::GetDataObject()
{
  return m_dataObj;
}

/// @brief  Create a new copy of *this class. No exceptions or thrown.
/// @param  T The object's type.
/// @return CDataObjBase* Pointer to the new object wrapper, NULL on new
///         allocation failure.
template<typename T>
CCVGUtilVariantType::CDataObjBase* CCVGUtilVariantType::CDataObj<T>::CreateCopyOfSelf()
{
  CDataObj *pCopy = new (std::nothrow) CDataObj<T>(m_dataObj);
  return pCopy;
}

/// @brief  Determine if *this object is a derived object from CDataObjBase.
///         Overidden from the base class which alway returns false.
/// @param  T The object's type.
/// @return bool True = *this is derived from CDataObjBase, false = base class.
template<typename T>
bool CCVGUtilVariantType::CDataObj<T>::GetIsDerivedClass() const
{
  return true;
}

/// @brief     Perform a bitwise copy of *this object.
/// @param     T The object's type.
/// @param[in] vrOther The otehr object.
template<typename T>
void CCVGUtilVariantType::CDataObj<T>::Duplicate(const CDataObj &vrOther)
{
  CDataObjBase::Copy(vrOther);
  m_dataObj = vrOther.m_dataObj;
}

/// @brief     Release any resources used by *this object.
/// @param     T The object's type.
template<typename T>
void CCVGUtilVariantType::CDataObj<T>::Destroy()
{
  CDataObjBase::Destroy();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @brief  Retrieve the data object from *this variant. Note if the variant 
///         object goes out of scope or is deleted then the data object is
///         deleted too.
/// @param  T The object's type.
/// @return T* Pointer to the object, NULL = data object not assigned to *this
///         variant object.
template<typename T>
T* CCVGUtilVariantType::Get() const
{
  if ((m_pDataObjectWrapper != nullptr) && (m_pDataObjectWrapper->GetIsDerivedClass()))
  {
    CDataObj<T> *pDataObj = static_cast<CDataObj<T> *>(m_pDataObjectWrapper);
    return &pDataObj->GetDataObject();
  }

  // Do not use a CDataObjBase object, use only a CDataObjBase derived objects
  return nullptr;
}

/// @brief      Assign to the variant an object of specified type. A copy is 
///             made of the object specified.
/// @param      T The object's type.
/// @param[in]  vrArg The object to store.
/// @return     bool True = success, false = failure unable to create new 
///             variant variable object.
template<typename T>
bool CCVGUtilVariantType::Set(const T &vrArg)
{
  m_pDataObjectWrapper = new (std::nothrow) CDataObj<T>(vrArg);
  return (m_pDataObjectWrapper != nullptr) ? true : false;
}

#endif // LPGPU2_CCVG_UTIL_VARIANTTYPE_H_INCLUDE