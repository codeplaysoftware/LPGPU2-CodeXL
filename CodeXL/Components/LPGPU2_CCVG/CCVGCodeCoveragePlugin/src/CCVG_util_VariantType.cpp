// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief CodeXL execution mode Code Coverage. LPGPU2 (== LP2) Code Coverage 
///        DLL plugin to sit along side other DLL plugins (execution modes)  
///        such as GPUProfilng or CPUProfiling.
///
/// CCVGUtilVariantType implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <CCVGCodeCoveragePlugin/Include/CCVG_util_VariantType.h>

/// @brief CDataObjBase constructor
CCVGUtilVariantType::CDataObjBase::CDataObjBase()
{
}

/// @brief CDataObjBase destructor
CCVGUtilVariantType::CDataObjBase::~CDataObjBase()
{
  Destroy();
}

/// @brief CDataObjBase copy constructor. It does nothing.
/// @param[in] vrOther The other object.
CCVGUtilVariantType::CDataObjBase::CDataObjBase(CDataObjBase &vrOther)
{
  GT_UNREFERENCED_PARAMETER(vrOther);
  // Do nothing
}

/// @brief        CDataObjBase move constructor. It does nothing.
/// @param[inout] vrOther The other object.
CCVGUtilVariantType::CDataObjBase::CDataObjBase(CDataObjBase &&vrOther)
{
  GT_UNREFERENCED_PARAMETER(vrOther);
  // Do nothing
}

/// @brief      CDataObjBase copy assignment.
/// @param[in]  vrOther The other object.
/// @return     CDataObjBase& *this object reference.
CCVGUtilVariantType::CDataObjBase& CCVGUtilVariantType::CDataObjBase::operator=(const CDataObjBase &vrOther)
{
  Copy(vrOther);
  return *this;
}

/// @brief        CDataObjBase move assignment.
/// @param[inout] vrOther The other object.
/// @return       CDataObjBase& *this object reference.
CCVGUtilVariantType::CDataObjBase& CCVGUtilVariantType::CDataObjBase::operator=(CDataObjBase &&vrOther)
{
  Copy(vrOther);
  vrOther.Destroy();
  return *this;
}

/// @brief Create a new copy of *this class. No exception is thrown.
/// @return vrOther The other object.
/// @return CDataObjBase* *this object pointer, NULL = new alloc failed.
CCVGUtilVariantType::CDataObjBase* CCVGUtilVariantType::CDataObjBase::CreateCopyOfSelf()
{
  // Override to implement copying of variant's data object
  return new (std::nothrow) CDataObjBase();
}

/// @brief  Determine if *this object is derived from CDataObjBase.
/// @return bool True = *this is derived from CDataObjBase, false = *this is 
///         of this base class.
bool CCVGUtilVariantType::CDataObjBase::GetIsDerivedClass() const
{
  // Override to in the derived class and return true
  return false;
}

/// @brief     Perform a bitwise copy of *this object.
/// @param[in] vrOther The other object.
void CCVGUtilVariantType::CDataObjBase::Copy(const CDataObjBase &vrOther)
{
  // Override to implement
  GT_UNREFERENCED_PARAMETER(vrOther);
}

/// @brief Release any resources use by *this object
void CCVGUtilVariantType::CDataObjBase::Destroy()
{
  // Do nothing - override to implement
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @brief     CDataObj copy constructor. 
/// @param[in] vrOther The other object.
template<typename T>
CCVGUtilVariantType::CDataObj<T>::CDataObj(CDataObj &vrOther)
{
  if (this == &vrOther)
  {
    return;
  }
  Copy(vrOther);
}

/// @brief        CDataObjBase move constructor. 
/// @param[inout] vrOther The other object.
template<typename T>
CCVGUtilVariantType::CDataObj<T>::CDataObj(CDataObj &&vrOther)
{
  if (this == &vrOther)
  {
    return;
  }
  Copy(vrOther);
  vrOther.Destroy();
}

/// @brief      CDataObj copy assignment.
/// @param[in]  vrOther The other object.
/// @return     CDataObj& *this object reference.
template<typename T>
CCVGUtilVariantType::CDataObj<T>& CCVGUtilVariantType::CDataObj<T>::operator=(const CDataObj &vrOther)
{
  if (this == &vrOther)
  {
    return *this;
  }
  Copy(vrOther);
  return *this;
}

/// @brief        CDataObj move assignment.
/// @param[inout] vrOther The other object.
/// @return       CDataObj& *this object reference.
template<typename T>
CCVGUtilVariantType::CDataObj<T>& CCVGUtilVariantType::CDataObj<T>::operator=(CDataObj &&vrOther)
{
  if (this == &vrOther)
  {
    return *this;
  }
  Copy(vrOther);
  vrOther.Destroy();
  return *this;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/// @brief CCVGUtilVariantType constructor.
CCVGUtilVariantType::CCVGUtilVariantType()
: m_pDataObjectWrapper(nullptr)
{
}

/// @brief CCVGUtilVariantType destructor.
CCVGUtilVariantType::~CCVGUtilVariantType()
{
  Destroy();
}

/// @brief CCVGUtilVariantType copy constructor.
/// @param[in] vrOther The other object.
CCVGUtilVariantType::CCVGUtilVariantType(CCVGUtilVariantType &vrOther)
: m_pDataObjectWrapper(nullptr)
{
  if (this == &vrOther)
  {
    return;
  }
  Copy(vrOther);
}

/// @brief CCVGUtilVariantType move constructor.
/// @param[in] vrOther The other object.
CCVGUtilVariantType::CCVGUtilVariantType(CCVGUtilVariantType &&vrOther)
: m_pDataObjectWrapper(nullptr)
{
  if (this == &vrOther)
  {
    return;
  }
  Copy(vrOther);
  vrOther.Destroy();
}

/// @brief CCVGUtilVariant copy assignment.
/// @prama[in] vrOther The other object.
/// @return CVVGUtilVariant& Return *this object.
CCVGUtilVariantType& CCVGUtilVariantType::operator=(const CCVGUtilVariantType &vrOther)
{
  if (this == &vrOther)
  {
    return *this;
  }
  Copy(vrOther);
  return *this;
}

/// @brief CCVGUtilVariant move assignment.
/// @prama[in] vrOther The other object.
/// @return CVVGUtilVariant& Return *this object.
CCVGUtilVariantType& CCVGUtilVariantType::operator=(CCVGUtilVariantType &&vrOther)
{
  if (this == &vrOther)
  {
    return *this;
  }
  Copy(vrOther);
  vrOther.Destroy();
  return *this;
}

/// @brief Release the resources used by *this object.
void CCVGUtilVariantType::Destroy()
{
  if (m_pDataObjectWrapper != nullptr)
  {
    delete m_pDataObjectWrapper;
    m_pDataObjectWrapper = nullptr;
  }
}

/// @brief Bitwise copy another data object to *this variant object.
/// @prama[in] vrOther The other object.
void CCVGUtilVariantType::Copy(const CCVGUtilVariantType &vrOther)
{
  Destroy();
  if (vrOther.m_pDataObjectWrapper != nullptr)
  {
    m_pDataObjectWrapper = vrOther.m_pDataObjectWrapper->CreateCopyOfSelf();
  }
}