/**
 * Copyright(c) Live2D Inc. All rights reserved.
 * Licensed under the MIT License. See LICENSE file in the project root for license information.
 */
#pragma once
#include <QFile>
#include <QQmlEngine>

namespace Live2D::Nizima::PluginApi {

class TextReader : public QObject {
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON
public:
  using QObject::QObject;
  Q_INVOKABLE QString read(const QString &path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return "";
    return file.readAll();
  }
};

}// namespace Live2D::Nizima::PluginApi
