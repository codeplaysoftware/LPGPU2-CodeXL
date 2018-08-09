// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Utility macros and functions used by the code implemented by Codeplay
///        in the Power Profiling module.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef PP_LPGPU2_UTILS_H_INCLUDE
#define PP_LPGPU2_UTILS_H_INCLUDE

#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTPowerProfiling/src/LPGPU2ppFnStatus.h>

/// @brief Create a new object allocating it on the heap and then run its
///        Initialise() method. Upon failure return the error status and
///        return from the function in which this is called. To be
///        used with LPGPU2 Qt widgets (i.e. classes developed by the LPGPU2
///        team which inherit widgets).
///
/// This method is to be used in conjunction with Illya's paradigm for error
/// handling and initialisation.
///
/// @param[out] obj Ptr to the ptr to the obj to be allocated
/// @param[in] args Ctor arguments
///
/// @return Whether allocation and initialisation succeeded
#define LPGPU2PPNewQtLPGPU2Widget(obj, ...)                         \
{                                                                   \
  using lpgpu2::PPFnStatus;                                         \
  PPFnStatus status = _LPGPU2PPNewQtLPGPU2Widget(obj, __VA_ARGS__); \
  if (status != PPFnStatus::success)                                \
  {                                                                 \
    return status;                                                  \
  }                                                                 \
}

/// @brief Create a new object allocating it on the heap.
///        Upon failure return the error status and
///        return from the function in which this is called. To be
///        used with PURELY Qt (i.e. classes NOT developed by the LPGPU2
///        team which are Qt widgets). It does not pass any parameters to the
///        object to be created.
///
/// This method is to be used in conjunction with Illya's paradigm for error
/// handling and initialisation.
///
/// @param[out] obj Ptr to the ptr to the obj to be allocated
///
/// @return Whether allocation and initialisation succeeded
#define LPGPU2PPNewQtWidgetNoParent(obj)      \
{                                             \
  using lpgpu2::PPFnStatus;                   \
  PPFnStatus status = LPGPU2PPNewNoInit(obj); \
  if (status != PPFnStatus::success)          \
  {                                           \
    return status;                            \
  }                                           \
}

/// @brief Create a new object allocating it on the heap.
///        Upon failure return the error status and
///        return from the function in which this is called. To be
///        used with PURELY Qt (i.e. classes NOT developed by the LPGPU2
///        team which are Qt widgets).
///
/// This method is to be used in conjunction with Illya's paradigm for error
/// handling and initialisation.
///
/// @param[out] obj Ptr to the ptr to the obj to be allocated
/// @param[in] args Ctor arguments
///
/// @return Whether allocation and initialisation succeeded
#define LPGPU2PPNewQtWidget(obj, ...)                           \
{                                                               \
  using lpgpu2::PPFnStatus;                                     \
  PPFnStatus status = LPGPU2PPNewNoInit(obj, __VA_ARGS__);      \
  if (status != PPFnStatus::success)                            \
  {                                                             \
    return status;                                              \
  }                                                             \
}

/// @brief Create a new object allocating it on the heap and then run its
///        Initialise() method. Upon failure return the error status. To be
///        used with Qt.
///
/// This method is to be used in conjunction with Illya's paradigm for error
/// handling and initialisation.
///
/// @see  DO NOT USE THIS. USE THE ASSOCIATED MACRO INSTEAD.
/// @param obj Ptr to the ptr to the obj to be allocated
/// @param args Ctor arguments for the object
///
/// @return Whether allocation and initialisation succeeded
template<typename T, typename... Args>
lpgpu2::PPFnStatus _LPGPU2PPNewQtLPGPU2Widget(T **obj, Args&&... args)
{
  using lpgpu2::PPFnStatus;

  static_assert(std::is_class<T>::value, "T is not a class!");

  *obj = new (std::nothrow) T(std::forward<Args>(args)...);
  GT_IF_WITH_ASSERT(obj != nullptr)
  {
    PPFnStatus status = (*obj)->Initialise();
    if (status == PPFnStatus::failure)
    {
      delete *obj;
      *obj = nullptr;
      return PPFnStatus::failure;
    }

    return PPFnStatus::success;
  }

  return PPFnStatus::failure;
}

/// @brief Create a new object allocating it on the heap.
///        Upon failure return the error status. Can be used directly
///
/// @param obj Ptr to the ptr to the obj to be allocated
/// @param args Ctor arguments for the object
///
/// @return Whether allocation and initialisation succeeded
template<typename T, typename... Args>
lpgpu2::PPFnStatus LPGPU2PPNewNoInit(T **obj, Args&&... args)
{
  using lpgpu2::PPFnStatus;

  static_assert(std::is_class<T>::value, "T is not a class!");

  *obj = new (std::nothrow) T(std::forward<Args>(args)...);
  GT_IF_WITH_ASSERT(obj != nullptr)
  {
    return PPFnStatus::success;
  }

  return PPFnStatus::failure;
}

#endif // PP_LPGPU2_UTILS_H_INCLUDE
