// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file LPGPU2ppSourceCodeTreeModel.h
///
/// @brief Data model for assembly code associated with performance counters
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.
#ifndef LPGPU2_PP_SOURCECODETREEMODEL_H 
#define LPGPU2_PP_SOURCECODETREEMODEL_H

// Qt:
#include <QAbstractItemModel>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtString.h>

#include <AMDTCpuProfiling/inc/SourceViewTreeItem.h>
#include <LPGPU2Database/LPGPU2Database/LPGPU2_db_DatabaseDataDefs.h>

#include <unordered_map>
#include <AMDTBaseTools/Include/gtVector.h>

namespace lpgpu2 {


/// @brief The possible columns in the Assembly view and model
struct SrcViewColumns
{
  enum Enum
  {
    row = 0,
    asmCol,
    percentage,

    enumCount
  };
};

/// @brief   Data model for assembly code listing views. This data model
///          represents performance, for a given counter, per assembly line
/// @date    19/02/2018
/// @author  Alberto Taiuti
// clang-format off
class ppSourceCodeTreeModel final : public QAbstractItemModel
{
  Q_OBJECT

// Methods
public:
  ppSourceCodeTreeModel(QWidget *parent = nullptr);
  ~ppSourceCodeTreeModel();

// Overridden methods
public:
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
      int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column,
      const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool setData(const QModelIndex& index, const QVariant& value,
      int role = Qt::EditRole) override;
  bool setHeaderData(int section, Qt::Orientation orientation,
      const QVariant& value, int role = Qt::EditRole) override;
  bool insertColumns(int position, int columns,
      const QModelIndex& parent = QModelIndex()) override;
  bool removeColumns(int position, int columns,
      const QModelIndex& parent = QModelIndex()) override;
  bool insertRows(int position, int rows,
      const QModelIndex& parent = QModelIndex()) override;
  bool removeRows(int position, int rows,
      const QModelIndex& parent = QModelIndex()) override;


// Methods
public:
  bool GetItemTopLevelIndex(const QModelIndex& index,
      int& indexOfTopLevelItem, int& indexOfTopLevelItemChild);
  void Clear();
  int topLevelItemCount();
  SourceViewTreeItem* topLevelItem(int index);
  void UpdateModel();
  void SetDisplayFormat(double  val, bool appendPercent, QVariant& data,
      const int precision);
  bool UpdateHeaders();
  bool BuildDisassemblyTree(const gtString &asmLines,
      const gtVector<db::ShaderAsmStat> &perc);

// Methods
private:
  SourceViewTreeItem* getItem(const QModelIndex& index) const;

// Attributes
private:
  SourceViewTreeItem* m_pRootItem = nullptr;
  QStringList m_headerCaptions;
  QStringList m_headerTooltips;
  QColor m_forgroundColor;

};

} // namespace lpgpu2

#endif // LPGPU2_PP_SOURCECODETREEMODEL_H
