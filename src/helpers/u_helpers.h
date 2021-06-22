// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef REPLACE_PLACEHOLDER_H
#define REPLACE_PLACEHOLDER_H

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

class UiHelpers {
 public:
  /**
   * @brief overlapPlaceholder Adds a component in the same position as the "placeholder" widget
   * @param placeholder The QWidget to overlap
   * @param replacement The QWidget that will overlap the placeholder
   * @param layout Where _both_ QWidgets should live
   */
  static void overlapPlaceholder(QWidget* placeholder, QWidget* replacement, QGridLayout* layout) {
    int row, col, rSpan, cSpan;
    auto widgetIndex = layout->indexOf(placeholder);
    if (widgetIndex == -1) {
      throw std::runtime_error("Placeholder is not contained in layout");
    }
    layout->getItemPosition(widgetIndex, &row, &col, &rSpan, &cSpan);
    layout->addWidget(replacement, row, col, rSpan, cSpan);
  }

  /**
   * @brief replacePlaceholder Replaces the _placeholder_ component with the _replacement_
   * component. The original/placeholder component is hidden and removed from the layout
   * @param placeholder The QWidget to remove
   * @param replacement The QWidget to add in the placeholder's position
   * @param layout Where the placeholder lives, and where the replacement will live
   */
  static void replacePlaceholder(QWidget* placeholder, QWidget* replacement, QGridLayout* layout) {
    overlapPlaceholder(placeholder, replacement, layout);
    placeholder->setVisible(false);
    layout->removeWidget(placeholder);
  }

  /**
   * @brief setComboBoxValue Sets a combobox's value based on the supplied _value_. Sets the value
   * to the proper index if found, otherwise sets the index to 0 if not found. Note: this does a
   * linear search for values, so may not be appropriate for all boxen.
   * @param box The source combobox
   * @param value The value to search for in the combobox
   */
  static void setComboBoxValue(QComboBox* box, const QString& value) {
    bool found = false;
    for (int i = 0; i < box->count(); i++) {
      if (box->itemData(i) == value) {
        box->setCurrentIndex(i);
        found = true;
        break;
      }
    }
    if (!found) {
      box->setCurrentIndex(0);
    }
  }
};

#endif  // REPLACE_PLACEHOLDER_H
