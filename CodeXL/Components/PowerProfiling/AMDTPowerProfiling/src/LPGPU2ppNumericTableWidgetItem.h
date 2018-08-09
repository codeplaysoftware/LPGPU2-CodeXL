// Copyright (C) 2002-2017 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppNumericTableWidgetItem.h
///
/// @brief Overrides the comparison operator of QTableWidgetItem to use 
///        numeric sorting rather than alphabetical.
///
/// @copyright
/// Copyright (C) 2002-2017 Codeplay Software Limited. All Rights Reserved.

#ifndef LPGPU2_PP_NUMERICTABLEWIDGETITEM_H_INCLUDE
#define LPGPU2_PP_NUMERICTABLEWIDGETITEM_H_INCLUDE

#include <QTableWidgetItem>
 
/// @brief   Overrides the comparison operator of QTableWidgetItem to use 
///          numeric sorting rather than alphabetical.
/// @author  Callum Fare
class LPGPU2ppNumericTableWidgetItem : public QTableWidgetItem 
{
public:
  bool operator<(const QTableWidgetItem &other) const override
  {
    return text().toDouble() < other.text().toDouble();
  }
};

#endif // LPGPU2_PP_NUMERICTABLEWIDGETITEM_H_INCLUDE
