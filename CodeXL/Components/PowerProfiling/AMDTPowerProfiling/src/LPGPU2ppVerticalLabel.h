// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Custom QLabel that displays its text vertically.
///
/// LPGPU2ppVerticalLabel interface.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#ifndef __LPGPU2PPVERTICALLABEL_H
#define __LPGPU2PPVERTICALLABEL_H

#include <qlabel.h>
#include <qpainter.h>

class LPGPU2ppVerticalLabel : public QLabel
{
	Q_OBJECT

public:
	explicit LPGPU2ppVerticalLabel(QWidget *parent = 0) : QLabel(parent) {}
	explicit LPGPU2ppVerticalLabel(const QString &text, QWidget *parent = 0) : QLabel(text, parent) {}

protected:
	virtual void paintEvent(QPaintEvent*) override;
};

#endif // __LPGPU2PPVERTICALLABEL_H