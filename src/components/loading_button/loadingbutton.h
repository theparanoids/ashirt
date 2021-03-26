// Copyright 2020, Verizon Media
// Licensed under the terms of MIT. See LICENSE file in project root for terms.

#ifndef LOADINGBUTTON_H
#define LOADINGBUTTON_H

#include <QPushButton>

#include "components/loading/qprogressindicator.h"

class LoadingButton : public QPushButton {
  Q_OBJECT

 public:
  explicit LoadingButton(QWidget* parent = nullptr, QPushButton* model = nullptr);
  explicit LoadingButton(const QString& text, QWidget* parent = nullptr,
                         QPushButton* model = nullptr);

  ~LoadingButton();

  void startAnimation();
  void stopAnimation();
  bool isAnimating();

 private:
  void showLabel(bool show);
  void init();

 protected:
  void resizeEvent(QResizeEvent* evt) override;

 private:
  QProgressIndicator* loading;

  QString label;
  bool showingLabel;
};

#endif  // LOADINGBUTTON_H
