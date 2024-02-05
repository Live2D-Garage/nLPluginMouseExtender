/**
 * Copyright(c) Live2D Inc. All rights reserved.
 * Licensed under the MIT License. See LICENSE file in the project root for license information.
 */
#pragma once
#include <QJsonObject>
#include <QMetaEnum>
#include <QtQml/qqmlregistration.h>

namespace Live2D::Nizima::PluginApi {

class Type : public QObject {
  Q_OBJECT
  QML_ELEMENT
  using QObject::QObject;

public:
  enum Enum {
    Invalid,
    Request,
    Response,
    Event,
    Error,
    UnknownType
  };
  Q_ENUM(Enum)

  static inline QString toString(Enum type) {
    return QMetaEnum::fromType<Enum>().valueToKey(type);
  }
  static inline Enum fromString(const QString &name) {
    int i = QMetaEnum::fromType<Enum>().keyToValue(name.toUtf8());
    if (i < 0) return Invalid;
    return static_cast<Enum>(i);
  }
};

class Method : public QObject {
  Q_OBJECT
  QML_ELEMENT
  using QObject::QObject;

public:
  enum Enum {
    Invalid,
    RegisterPlugin,
    EstablishConnection,
    GetConnectionStatus,
    RegisterPluginPath,
    DelayTest,
    // Event
    NotifyLaunchButtonClicked,
    NotifyEnabledChanged,
    NotifyCurrentSceneChanged,
    NotifyCurrentModelChanged,
    NotifyFrameUpdated,
    NotifyScenesChanged,
    NotifyModelsChanged,
    NotifyItemsChanged,
    // Parameter
    GetLiveParameters,
    GetLiveParameterValues,
    SetLiveParameterValues,
    InsertLiveParameters,
    GetCubismParameters,
    GetCubismParameterValues,
    SetCubismParameterValues,
    ResetCubismParameterValues,
    // Scene
    GetScenes,
    GetCurrentSceneId,
    // Load Model
    GetRegisteredModels,
    GetRegisteredModelIcons,
    RegisterModel,
    AddModel,
    ChangeModel,
    RemoveModel,
    // Control Model
    GetModels,
    GetCurrentModelId,
    SetModelColor,
    GetPartsTree,
    GetParts,
    SetPartsColor,
    GetDrawables,
    SetDrawablesColor,
    TriggerModelHotkey,
    MoveModel,
    ResetPose,
    // Motion/Expression
    GetMotions,
    StartMotion,
    StopMotion,
    GetExpressions,
    StartExpression,
    StopExpression,
    StopAllExpressions,
    // Load Item
    GetRegisteredItems,
    AddItem,
    RemoveItem,
    // Control Item
    GetItems,
    MoveItem,

    UnknownMethod
  };
  Q_ENUM(Enum)

  static inline QString toString(Enum type) {
    return QMetaEnum::fromType<Enum>().valueToKey(type);
  }
  static inline Enum fromString(const QString &name) {
    int i = QMetaEnum::fromType<Enum>().keyToValue(name.toUtf8());
    if (i < 0) return Invalid;
    return static_cast<Enum>(i);
  }
};

struct Message {
  Q_GADGET
public:
  static constexpr auto currentVersion = "1.0.0";

  QString version = currentVersion;
  QDateTime timestamp = QDateTime::currentDateTime();
  QString id;
  Type::Enum type;
  Method::Enum method;
  QJsonObject data;

  QJsonObject toJson() const {
    return {{"nLPlugin", version},
            {"Timestamp", timestamp.toMSecsSinceEpoch()},
            {"Id", id},
            {"Type", Type::toString(type)},
            {"Method", Method::toString(method)},
            {"Data", data}};
  }
  static inline Message fromJson(const QJsonObject &obj) {
    return {.version = obj["nLPlugin"].toString(),
            .timestamp = QDateTime::fromMSecsSinceEpoch(obj["Timestamp"].toInteger()),
            .id = obj["Id"].toString(),
            .type = Type::fromString(obj["Type"].toString()),
            .method = Method::fromString(obj["Method"].toString()),
            .data = obj["Data"].toObject()};
  }
};

}// namespace Live2D::Nizima::PluginApi
