// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief Custom QLabel that displays its text vertically.
///
/// LPGPU2ppVerticalLabel implementation.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

#include "LPGPU2ppVerticalLabel.h"

/// @brief  Overrides QWidget's paintEvent method to rotate and translate
///         the QLabel text so that is displayed vertically.
void LPGPU2ppVerticalLabel::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setPen(Qt::black);
	painter.setBrush(Qt::Dense1Pattern);

	// Set the text font to bold
	QFont font = painter.font();
	font.setBold(true);
	painter.setFont(font);

	// Set the height of this label to be equal to its text width
	setFixedHeight(painter.fontMetrics().width(text()));

	// Calculate the dimensions of the text that will be drawn
	QFontMetrics fm = painter.fontMetrics();
	int textWidth = fm.width(text());
	int textHeight = fm.height();

	// Rotate the text painter by 270 degrees, which pivots on the top-left
	// corner of the rect. Account for this pivot by translating "down" by the
	// text width.
	painter.translate(0, textWidth);
	painter.rotate(270);

	// Further translate the rotated text so that it is centered
	// horizontally
	painter.translate(0, (width() - textHeight) / 2);

	// Draw the vertical label, accounting for the drawText pivot
	// being the bottom-left corner of the rect
	int textX = 0;
	int textY = textHeight;
	painter.drawText(textX, textY, text());
}